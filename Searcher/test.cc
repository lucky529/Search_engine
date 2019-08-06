#include"Serarcher.hpp"
using namespace std;
int main()
{
    //searcher::Index index;
    //index.Build("../Data/Tmp/raw_input");
    //auto* inverted_list = index.GetInvertedList("filesystem");
    //for(auto weight : *inverted_list){
    //    cout << "id : " << weight.doc_id << "weight : "<<weight.weight <<endl;
    //    const auto* doc_info = index.GetDocInfo(weight.doc_id);
    //    cout << "url :" << doc_info->url <<endl;
    //}
    searcher::Searcher searcher;
    bool rret = searcher.Init("../Data/Tmp/raw_input");

    string query = "filesystem";
    string ret;
    searcher.Search(query, ret);
    cout << ret <<endl;
    return 0;
}
