#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<map>
#include<vector>
#include<list>
#include<fstream>
#include"util.hpp"
#include"mysql.h"


/*
* book_model.hpp:将数据库中的书籍信息加载起来，供服务器随时使用
*/

struct Book{
    std::string id;
    std::string name; 
    std::string isbn;
    std::string author;
    std::string owner;
    std::string borrower;
    std::string islend;
    std::string num;
    std::string uid;
    std::string lend_time;
    std::string back_time;

    std::string dir; //书籍主页对应的目录,书籍描述，描述格式
    std::string desc;//书籍描述
    std::string header;//书籍描述格式
    std::string img;//书籍图片

    
   //std::list<std::string> applist; //预约排队表
};

class BookModel{
    private:
        std::map<int,Book>model;
    public:

        //将书库表中的信息加载到内存，插入哈希表中
        bool Load(){
            LOG(INFO)<<"book tables begins"<<std::endl;
            model.clear();
            
            MYSQL_RES *result;
            MYSQL_ROW row; 
            int num;
            int i;

            //1. 拼接sql语句
            std::string sql="select * from BookBase";
            //2. 连接数据库
            MYSQL *conn = mysql_init(nullptr);
            mysql_set_character_set(conn,"utf8");
            if(nullptr == mysql_real_connect(conn,"127.0.0.1","http_test","12345678","http_test",3306,nullptr,0)){
              LOG(WARING)<<"connect fail!"<<std::endl;
              return false;
            }
            LOG(INFO)<<"connect sucess"<<std::endl;
            LOG(INFO)<<"query: "<<sql<<std::endl;
            //3. 读数据库
            if(mysql_query(conn,sql.c_str())){
                LOG(WARING)<<"select fail!"<<std::endl;
                return false;
            }
            LOG(INFO)<<"select sucess"<<std::endl;
            result = mysql_store_result(conn);//将查询结果存入result
            num = mysql_num_fields(result);   //将结果集的列数存储起来
            //4. 根据解析结果拼装 Book 结构体

            Book b;
            

            while((row = mysql_fetch_row(result))){
                std::vector<std::string>tokens;
                for(i=0;i<num;i++){
                    std::cout<<row[i]<<"\t";
                    tokens.push_back(row[i]);
                }
                if(tokens.size()!=11){
                     LOG(ERROR)<<"bookbase format error!\n";
                     continue;
                }
                std::cout<<std::endl;
                //存入总书库中的信息
                b.id=tokens[0];
                b.name=tokens[1];
                b.author=tokens[2];
                b.isbn=tokens[3];
                b.owner=tokens[4];
                b.borrower=tokens[5];
                b.islend=tokens[6];
                b.num=tokens[7];
                b.uid=tokens[8];
                b.lend_time=tokens[9];
                b.back_time=tokens[10];

                model[std::atoi(b.id.c_str())]=b;
            }
            mysql_free_result(result);
            mysql_close(conn);

            // //加载 接口文件 到内存
            std::ifstream file("./book_data/book_config.cfg");
             if(!file.is_open()){
                LOG(WARING)<<"open book_config fail!"<<std::endl;
                return false;
            }
            //按行读取 book_config文件
            std::string line;
            while(std::getline(file,line)){
                std::vector<std::string>tokens;
                StringUtil::Split(line,"\t",&tokens);
                if(tokens.size()!=2){
                    LOG(ERROR)<<"config file format error!\n";
                    continue; 
                }
                b.id=tokens[0];
                b.dir=tokens[1];

                FileUtil::Read(b.dir+"/desc.txt",&b.desc);
                FileUtil::Read(b.dir+"/header.txt",&b.header);
                FileUtil::Image_Read(b.dir+"/img.jpg",&b.img);

                model[std::atoi(b.id.c_str())].dir=b.dir;
                model[std::atoi(b.id.c_str())].desc=b.desc;
                model[std::atoi(b.id.c_str())].header=b.header;
                model[std::atoi(b.id.c_str())].img=b.img;
            }
            std::cout<<b.img<<std::endl;
            // for(auto e:model){
            //     LOG(INFO)<<e.second.desc<<" "<<e.second.header<<std::endl;
            // }

            LOG(INFO)<<"load "<<model.size()<<" books\n";
            return true;
        }

        //获取所有书籍
        bool GetAllBooks(std::vector<Book>* books)const{
            books->clear();
            for(const auto& kv :model){
                books->push_back(kv.second);
            }
            return true;
        }
        //按照id查找书籍
        bool GetBook(const std::string& id, Book* q)const{
            auto pos = model.find(std::atoi(id.c_str()));
            if(pos == model.end()){
                return false;
            }
            *q=pos->second;
            return true;
        }
        
        //按照isbn查找书籍
        bool GetBookByISBN(const std::string& isbn,Book*q)const{
           for(auto e: model){
                if(e.second.isbn == isbn){
                    *q=e.second;
                    return true;
                }
           }
           return false;
        }

        //根据用户名获取我的书库
        bool GetMyBooksByUname(const std::string& owenr,std::vector<Book>*books)const{
            books->clear();
            for(const auto& kv :model){
                if(kv.second.owner==owenr)
                    books->push_back(kv.second);
            }
            return true;
        }

        //根据用户id获取我的书库
        bool GetMyBooksById(const std::string& uid,std::vector<Book>*books)const{
            books->clear();
            for(const auto& kv :model){
                if(kv.second.uid==uid)
                    books->push_back(kv.second);
            }
            return true;
        }

        //获取我的借出
        bool GetMyLend(const std::string& uid,std::vector<Book>*books)const{
            books->clear();
            for(const auto& kv :model){
                if(kv.second.uid == uid && kv.second.islend =="1"){
                    books->push_back(kv.second);
                }
            }
            return true;
        }

         //获取我的已借
        bool GetMyBorrow(const std::string& borrower,std::vector<Book>*books)const{
            books->clear();
            for(const auto& kv :model){
                if(kv.second.borrower == borrower){
                    books->push_back(kv.second);
                }
            }
            return true;
        }
        //按照书名查找书籍
        bool GetSearchBooksByBname(const std::string& name,std::vector<Book>*books)const{
            books->clear();
            for(const auto& kv :model){
                if(kv.second.name==name)
                    books->push_back(kv.second);
            }
            return true;
        }

        //按照isbn查找书籍
        bool GetSearchBooksByISBN(const std::string& isbn,std::vector<Book>*books)const{
            books->clear();
            for(const auto& kv :model){
                if(kv.second.isbn==isbn)
                    books->push_back(kv.second);
            }
            return true;
        }

        //按照作者查找书籍
        bool GetSearchBooksByAuthor(const std::string& author,std::vector<Book>*books)const{
            books->clear();
            for(const auto& kv :model){
                if(kv.second.author==author)
                    books->push_back(kv.second);
            }
            return true;
        }
        

        //按照搜索类型和内容查找用户
        bool GetUserSearchResult(const std::string& sfrom ,const std::string& search,std::vector<Book>*result)const{
           result->clear();
           if(sfrom == "name"){
               GetSearchBooksByBname(search,result);
           }
           else if(sfrom == "author"){
               GetSearchBooksByAuthor(search,result);
           }
           else if(sfrom == "isbn"){
               GetSearchBooksByISBN(search,result);
           }
           else if(sfrom =="owner"){
               GetMyBooksByUname(search,result);
           }
           else{
               LOG(ERROR)<<"unnamed search from"<<std::endl;
           }

          
           return false;
        }
        //插入数据库
        bool InsertBook(std::string sql){
            //2. 连接数据库
            MYSQL *conn = mysql_init(nullptr);
            mysql_set_character_set(conn,"utf8");
            if(nullptr == mysql_real_connect(conn,"127.0.0.1","http_test","12345678","http_test",3306,nullptr,0)){
              LOG(WARING)<<"connect fail!"<<std::endl;
              return false;
            }
            LOG(INFO)<<"connect sucess"<<std::endl;
            LOG(INFO)<<"query: "<<sql<<std::endl;
            //3. 写数据库
            int ret;
            if(ret = mysql_query(conn,sql.c_str())){
                LOG(WARING)<<"ret: "<< ret <<" write fail!"<<std::endl;
                return false;
            }
            LOG(INFO)<<"write sucess"<<std::endl;
            return true;
        }

        //更新数据库
        bool UpDateBook(std::string sql){
            //2. 连接数据库
            MYSQL *conn = mysql_init(nullptr);
            mysql_set_character_set(conn,"utf8");
            if(nullptr == mysql_real_connect(conn,"127.0.0.1","http_test","12345678","http_test",3306,nullptr,0)){
              LOG(WARING)<<"connect fail!"<<std::endl;
              return false;
            }
            LOG(INFO)<<"connect sucess"<<std::endl;
            LOG(INFO)<<"query: "<<sql<<std::endl;
            //3. 写数据库
            int ret;
            if(ret = mysql_query(conn,sql.c_str())){
                LOG(WARING)<<"ret: "<< ret <<" update fail!"<<std::endl;
                return false;
            }
            LOG(INFO)<<"update sucess"<<std::endl;
            return true;
        }

        


};
