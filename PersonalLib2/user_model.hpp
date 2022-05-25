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

struct User{
    std::string uid;
    std::string uname; 
    std::string phone;
    std::string email;
    std::string adress;
    std::string passwd;

    std::string dir;
    std::string desc;
    std::string header;
    std::string img;
};

class UserModel{
    private:
        std::map<int,User>model;
    public:
        //将书库表中的信息加载到内存，插入哈希表中
        bool Load(){
            LOG(INFO)<<"user tables begins"<<std::endl;
            model.empty();

            MYSQL_RES *result;
            MYSQL_ROW row; 
            int num;
            int i;

            //1. 拼接sql语句
            std::string sql="select * from user";
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

            User u;
        
            while((row = mysql_fetch_row(result))){
                std::vector<std::string>tokens;
                for(i=0;i<num;i++){
                    std::cout<<row[i]<<"\t";
                    tokens.push_back(row[i]);
                }
                if(tokens.size()!=6){
                     LOG(ERROR)<<"bookbase format error!\n";
                     continue;
                }
                std::cout<<std::endl;
                //存入总书库中的信息
                u.uid=tokens[0];
                u.uname=tokens[1];
                u.phone=tokens[2];
                u.email=tokens[3];
                u.adress=tokens[4];
                u.passwd=tokens[5];

                model[std::atoi(u.uid.c_str())]=u;
            }
            mysql_free_result(result);
            mysql_close(conn);

            // //加载 接口文件 到内存
            std::ifstream file("./user_data/user_config.cfg");
             if(!file.is_open()){
                LOG(WARING)<<"open user_config fail!"<<std::endl;
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
                u.uid=tokens[0];
                u.dir=tokens[1];

                FileUtil::Read(u.dir+"/desc.txt",&u.desc);
                FileUtil::Read(u.dir+"/header.txt",&u.header);
                FileUtil::Image_Read(u.dir+"/img.jpg",&u.img);

                model[std::atoi(u.uid.c_str())].dir=u.dir;
                model[std::atoi(u.uid.c_str())].desc=u.desc;
                model[std::atoi(u.uid.c_str())].header=u.header;
                model[std::atoi(u.uid.c_str())].img=u.img;
            }
            std::cout<<u.img<<std::endl;
            // for(auto e:model){
            //     LOG(INFO)<<e.second.desc<<" "<<e.second.header<<std::endl;
            // }
            LOG(INFO)<<"load "<<model.size()<<" users\n";
            return true;
        }
        //获取所有用户
        bool GetAllUsers(std::vector<User>* users)const{
            users->clear();
            for(const auto& kv :model){
                users->push_back(kv.second);
            }
            return true;
        }

        //按照uid查找用户
        bool GetUser(const std::string& uid, User* q)const{
            auto pos = model.find(std::atoi(uid.c_str()));
            if(pos == model.end()){
                return false;
            }
            *q=pos->second;
            return true;
        }
        //按照名字查找用户
        bool GetUserByName(const std::string& uname, User* q)const{
            for(auto e: model){
                if(e.second.uname == uname){
                    *q=e.second;
                    return true;
                }
           }
           return false;
        }
        //按照电话查找用户
        bool GetUserByPhone(const std::string& phone,User*q)const{
           for(auto e: model){
                if(e.second.phone == phone){
                    *q=e.second;
                    return true;
                }
           }
           return false;
        }

        //按照uid和 名字 和 密码查找用户
        bool GetUserVerify(const std::string& uname ,const std::string& passwd,User*p)const{
           for(auto e: model){
                if(e.second.uname == uname){
                    if(e.second.passwd == passwd){
                        *p=e.second;
                        return true;
                    }
                    LOG(ERROR)<<"密码错误"<<std::endl;return false;
                    
                }
           }
           LOG(ERROR)<<"用户不存在"<<std::endl;
           return false;
        }

        //插入用户数据库
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
            if(mysql_query(conn,sql.c_str())){
                LOG(WARING)<<"write mysql fail!"<<std::endl;
                return false;
            }
            LOG(INFO)<<"write nysql sucess"<<std::endl;
            return true;
        }


        
};