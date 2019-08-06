#pragma once
#include<string>
#include<unordered_map>
#include<iostream>
#include<algorithm>
#include<fstream>
#include<json/json.h>
#include"../Common/Util.hpp"
#include"cppjieba/Jieba.hpp"

//构建索引模块和搜索模块
namespace searcher{

const char* const DICT_PATH = "../Jieba_dict/jieba.dict.utf8";
const char* const HMM_PATH = "../Jieba_dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../Jieba_dict/user.dict.utf8";
const char* const IDF_PATH = "../Jieba_dict/idf.utf8";
const char* const STOP_WORD_PATH = "../Jieba_dict/stop_words.utf8";

struct DocInfo{
    uint64_t doc_id;
    std::string title;
    std::string content;
    std::string url;
};

struct Weight{
    uint64_t doc_id;
    int weight;//为了后面进行排序做准备，采用词频进行计算
    std::string key;
};

typedef std::vector<Weight> InvertedList;

//通过这个类描述索引模块
class Index{
    private:
        //知道id获取对应的文档内容
        //使用vector下标获取文档id
        std::vector<DocInfo> forward_index;
        //知道词，获取到对应的id列表
        std::unordered_map<std::string, InvertedList> inverted_index;
        cppjieba::Jieba jieba;
    public:
        Index():jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH)
        {}
        //读取 raw_input文件，在内存中构建索引
        bool Build(const std::string& input_path)
        {
            std::cout << "Index Build Start!" << std::endl;
            //1.按行读取文件内容
            std::ifstream file(input_path.c_str());
            if(!file.is_open()){
                std::cout << "input_paht open failed " << input_path << std::endl;
                return false;
            }
            std::string line;
            while(std::getline(file, line)){
                //2.构造 DocInfo对象，跟新正排索引数据，对读到的每一行文件进行解析，并插入vector中
                const DocInfo* doc_info = this->BuildForward(line);
                //3.更新倒排索引数据
                this->BuildInverted(*doc_info);
            }
            std::cout << "Index Build End!" << std::endl;
            file.close();
            return true;
        }
        //查正排，给定id找到文档内容
        const DocInfo* GetDocInfo(uint64_t doc_id) const
        {
            if(doc_id >= forward_index.size()){
                return NULL;
            }
            return &forward_index[doc_id];
        }
        //查倒排，给定词，找到这个词在哪些文档出现过
        const InvertedList* GetInvertedList(const std::string& key) const
        {
            auto pos = inverted_index.find(key);
            if(pos == inverted_index.end()){
                return NULL;
            }
            return &pos->second;
        }
        void CutWord(const std::string& input, std::vector<std::string>& output)
        {
            jieba.CutForSearch(input, output);
        }
        const DocInfo* BuildForward(std::string& line)
        {
            //1.对这一些进行切分
            std::vector<std::string> tockens;
            //借助boost进行切分
            StringUtil::Split(line, tockens, "\3");
            if(tockens.size() != 3){
                std::cout << "tokens not ok" << std::endl;
                return NULL;
            }
            //2.构造一个DcoInfo对象
            DocInfo doc_info;
            doc_info.doc_id = forward_index.size();
            doc_info.title = tockens[0];
            doc_info.url = tockens[1];
            doc_info.content = tockens[2];
            forward_index.push_back(doc_info);
            return &forward_index.back();
        }
        void BuildInverted(const DocInfo& doc_info)
        {
            //1.先对当前的doc_info进行分词，对正文分词，对标题分词
            std::vector<std::string> title_tokens;
            CutWord(doc_info.title, title_tokens);
            std::vector<std::string> content_tokens;
            CutWord(doc_info.content, content_tokens);
            //2.对doc_info中的标题和正文进行词频统计
            struct WordCnt{
                int title_cnt;
                int content_cnt;
            };
            //用一个哈希表完成词频的统计
            std::unordered_map<std::string, WordCnt> word_cnt;
            for(std::string word : title_tokens){
                boost::to_lower(word);
                ++word_cnt[word].title_cnt;
            }
            for(std::string word : content_tokens){
                boost::to_lower(word);
                ++word_cnt[word].content_cnt;
            }
            //3.遍历分词结果，在倒排索引中查找
            for(const auto& word_pair : word_cnt){
                Weight weight;
                weight.weight = 10 * word_pair.second.title_cnt + word_pair.second.content_cnt;
                weight.doc_id = doc_info.doc_id;
                weight.key = word_pair.first;
                //4.如果改分词结果在倒排中不存在，那就构建新的键值对
                InvertedList& inverted_list = inverted_index[word_pair.first];
                //5.如果该分词在倒排中存在，找到对应的值（vector），构建一个新的weight对象插入到vector中
                inverted_list.push_back(weight);
            }
        }
};

//搜索模块
class Searcher{
    private:
        Index* index;
    private:
        std::string GetDesc(const std::string& content, const std::string& key)
        {
            //找一下这个词的位置，往前后截取结果
            size_t pos = content.find(key);
            if(pos == std::string::npos){
                //该词在正文中不存在
                if(content.size() < 120){
                    return content;
                }
                else{
                    return content.substr(0, 120) + "...";
                }
            }
            size_t beg = pos - 60 ? 0 : pos - 60;
            if(beg + 120 >= content.size()){
                return content.substr(beg);
            }
            else{
                return content.substr(beg, 120) + "...";
            }
            return "";
        }
    public:
        Searcher(): index(new Index())
        {

        }
        //加载索引
        bool Init(const std::string& input_path)
        {
            return index->Build(input_path);
        }
        //通过特定的格式在 result 字符串中表示搜索结果
        bool Search(const std::string& query, std::string& json_result)
        {
            //1.分词
            std::vector<std::string> tokens;
            index->CutWord(query, tokens);
            //2.触发：针对结果查倒排索引，找到哪些文档是具有相关性的
            std::vector<Weight> all_token_result;
            for(std::string word : tokens){
                boost::to_lower(word);
                auto* inverted_list = index->GetInvertedList(word);
                if(inverted_list == nullptr){
                    continue;
                    //不存在当前词存在的情况
                }
                //此处进一步考虑不同的分词结果对应相同id的情况，合并有序链表
                all_token_result.insert(all_token_result.end(), \
                        inverted_list->begin(), inverted_list->end());
            }
            //3.把这些结果按照一定规则排序
            //使用拉姆达表达式
            std::sort(all_token_result.begin(), all_token_result.end(), [](const Weight& w1, const Weight& w2){
                        return w1.weight > w2.weight;
                    });
            //4.查正排找到每个搜索结果的标题，正文，url
            Json::Value results;
            for(const auto& weight : all_token_result){
                const auto* doc_info = index->GetDocInfo(weight.doc_id);
                if(doc_info == nullptr){
                    continue;
                }
                //使用现成的jsoncpp来进行构造
                Json::Value result;
                result["title"] = doc_info->title;
                result["url"] = doc_info->url;
                result["desc"] = GetDesc(doc_info->content, weight.key);
                results.append(result);
            }
            Json::FastWriter writer;
            json_result = writer.write(results);
            return true;
        }
        ~Searcher()
        {
            delete index;
        }
};

}// end searcher
