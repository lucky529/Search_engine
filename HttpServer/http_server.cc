#include <httplib.h>
#include"Serarcher.hpp"
#include"../Common/Util.hpp"

int main(void)
{
    std::string ss;
    FileUtil::Read("./web/index.html", ss);
    searcher::Searcher s;
    bool ret = s.Init("../Data/Tmp/raw_input");
    if(!ret){
        std::cout << "Searcher Init failed" << std::endl;
        return 1;
    }
    using namespace httplib;
    Server svr;
    svr.Get("/search", [&s](const Request& req, Response& res) {
            std::string query = req.get_param_value("query");
             std::string result;
             s.Search(query, result);
            res.set_content(result, "text/plain");
    });

    svr.set_base_dir("./web"); 
    svr.listen("0.0.0.0", 8080);
    return 0;
}
