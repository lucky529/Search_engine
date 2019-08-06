# search-engine
基于boost文档的搜索引擎

## Common目录

Common是一个工具目录，此目录中Util.hpp头文件中包含对文件的读写操作，以及使用boost库中字符分割函数对字符串进行分割

## Data目录

Data是一个输入输出文件管理目录，目录中html目录包含待处理的约5000条boost html，另一个tmp目录中的raw_input文件存放boost html的处理结果

## HttpServer目录

HttpServer目录中包含最后在服务器上运行的可执行文件，此服务器使用第三方库httplib，一个非常轻量化的服务器，服务器的演示移步至此文件下的http_server.cc
另外web目录下包含要返回的搜索html界面

## Jieba_dict目录

不用太多关心此目录，目录中存放分词的相关词典，直接调用即可

## Parser目录

目录中单独实现了分词的模块，模块中使用了第三方库cppjieba，这个库能将一个词按照语言分解成不同的结果

## Searcher目录

Searcher目录中具体实现了搜索引擎模块，包含对分词构建倒排和正排索引，以及根据分词结果在已经构建好的索引中进行搜索


ps：这里包含对各个文件的简要描述，具体引擎实现过程讲解移步至笔者的博客
