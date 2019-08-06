//数据处理模块
//把boost文档中涉及到的html进行处理
//1.去标签 2.把文件进行合并，把文档中涉及到的N个HTML的内容合并一个文本文件,生成的结果是一个大文件，每一行对应boost文档的一个html
//3.对文档的结构进行分析，提取出文档的标题，正文，目标url


#include<string>
#include<vector>
#include<iostream>
#include<fstream>
#include"../Common/Util.hpp"
//遍历目录和枚举所用到的头文件
#include<boost/filesystem/path.hpp>
#include<boost/filesystem/operations.hpp>
const std::string input_path = "../Data/Input/";
const std::string out_path = "../Data/Tmp/raw_input";


//doc指的是文档，也就是待搜索的html

struct DocInfo{
    std::string title;
    std::string content;
    std::string url;
};

bool EnumFile(const std::string& input_path, std::vector<std::string>& file_list)
{
    namespace fs = boost::filesystem;
    fs::path root_path(input_path);
    if(!fs::exists(root_path)){
        std::cout << "input_paht not exists" <<std::endl;
        return false;
    }
    fs::recursive_directory_iterator end_iter;
    for(fs::recursive_directory_iterator iter(root_path); iter != end_iter; ++iter){
        //需要剔除目录和图片，根据扩展名只保留html
        if(!fs::is_regular_file(*iter)){//只保留普通文件
            continue;
        }
        if(iter->path().extension() != ".html"){
            continue;
        }
        file_list.push_back(iter->path().string());
    }
    return true;
}

bool ParseTitle(const std::string& html, std::string& title)
{
    //1.先查找<title>标签
    size_t beg = html.find("<title>");
    if(beg == std::string::npos){
        std::cout << "beg title not found" << std::endl;
        return false;
    }
    //2.再查找</title>标签
    size_t end = html.find("</title>");
    if(end == std::string::npos){
        std::cout << "end title not found" << std::endl;
        return false;
    }
    beg += std::string("<title>").size();
    if(beg > end){
        std::cout << "beg > end error" << std::endl;
        return false;
    }
    //3.最后使用substr截图内容
    title = html.substr(beg, end - beg);
    return true;
}

bool ParseContent(const std::string& html, std::string& content)
{
    //一个字符一个字符处理 < 认为标签开始，接下来的字符就舍弃
    bool is_content = true;
    for(auto c : html){
        if(is_content){
            if(c == '<'){
                is_content = false;
            }
            else{
                if(c == '\n') c = ' ';
                content.push_back(c);
            }
        }
        else{
            if(c == '>'){
                is_content = true;
            }
        }
    }
    return true;
}

//boost文档url有一个统一的前缀
//url的后半部分可以从该文档的路径中解析出来
bool ParseUrl(const std::string& file_path, std::string& url)
{
    std::string prefix = "https://www.boost.org/doc/libs/1_70_0/doc/";
    std::string tail = file_path.substr(input_path.size());
    url = prefix + tail;
    return true;
}

bool ParseFile(const std::string& file_path, DocInfo* doc_info)
{
    //1.打开文件，读取文件内容
    std::string html;
    bool ret = FileUtil::Read(file_path, html);
    if(!ret){
        std::cout << "read file failed" <<std::endl;
        return false;
    }
    //2.解析标题
    ret = ParseTitle(html, doc_info->title);
    if(!ret){
        std::cout << "parse title error" <<std::endl;
        return false;
    }
    //3.解析正文，并且去除html的标签
    ret = ParseContent(html, doc_info->content);
    if(!ret){
        std::cout << "parse content error" <<std::endl;
        return false;
    }
    //4.解析出url
    ret = ParseUrl(file_path, doc_info->url);
    if(!ret){
        std::cout << "parse url error" <<std::endl;
        return false;
    }
    return true;
}

//c++中ofstream 和 ifstream都是紧张拷贝的
void WriteOutput(DocInfo& doc_info, std::ofstream& file)
{
    std::string line = doc_info.title + "\3" + doc_info.url + "\3" + doc_info.content + "\n";
    file.write(line.c_str(), line.size());
}
int main()
{
    //1.枚举出路径中所有的html文档路径
    std::vector<std::string> file_list;
    bool ret = EnumFile(input_path, file_list);
    if(!ret){
        std::cerr << "enum file error" << std::endl;
        return 1;
    }
    for(auto e : file_list)
    {
        std::cout << e << std::endl;
    }
    std::ofstream output_file(out_path.c_str());
    if(!output_file.is_open()){
        std::cout << "open file error" << std::endl;
        return 1;
    }
    //2.依次处理每个枚举出的路径，对文件进行分析，分析中标题，正文，url，并且进行去标签
    for(const auto& path : file_list){
        DocInfo info;
        ret = ParseFile(path, &info);//对文件进行解析
        if(!ret){
            std::cerr << "parse file error"<< path << std::endl;
            continue;
        }
        //3.把分析结果按照一行的形式写到输出文件中
        WriteOutput(info, output_file);
    }

    output_file.close();
    return 0;
}

