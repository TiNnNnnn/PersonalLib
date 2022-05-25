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

struct Comm{
    std::string cid;
    std::string bid; 
    std::string uid;
    std::string content;
    std::string Responser;
    std::string Respondent;
    std::string ctime;
};

class CommModel{
    private:
        std::map<int,Comm>model;
    public:
        //将评论表中的信息加载到内存，插map中
        bool Load(){
            LOG(INFO)<<"user tables begins"<<std::endl;
            model.empty();

            MYSQL_RES *result;
            MYSQL_ROW row; 
            int num;
            int i;

            //1. 拼接sql语句
            std::string sql="select * from Comments";
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

            Comm c;
        
            while((row = mysql_fetch_row(result))){
                std::vector<std::string>tokens;
                for(i=0;i<num;i++){
                    std::cout<<row[i]<<"\t";
                    tokens.push_back(row[i]);
                }
                if(tokens.size()!=7){
                     LOG(ERROR)<<"Comments format error!\n";
                     continue;
                }
                std::cout<<std::endl;
                //存入总书库中的信息
                c.cid=tokens[0];
                c.bid=tokens[1];
                c.uid=tokens[2];
                c.content=tokens[3];
                c.Responser=tokens[4];
                c.Respondent=tokens[5];
                c.ctime=tokens[6];

                model[std::atoi(c.cid.c_str())]=c;
            }
            mysql_free_result(result);
            mysql_close(conn);

            LOG(INFO)<<"load "<<model.size()<<" comments\n";
            return true;
        }

        //获取所有评论
        bool GetAllComms(std::vector<Comm>* comms)const{
            comms->clear();
            for(const auto& kv :model){
                comms->push_back(kv.second);
            }
            return true;
        }

        //根据书籍id 获取评论
        bool GetCommsByBid(const std::string& bid,std::vector<Comm>*comms)const{
            comms->clear();
            for(const auto& kv :model){
                if(kv.second.bid==bid)
                    comms->push_back(kv.second);
            }
            return true;
        }

        //插入评论数据库
        bool InsertComments(std::string sql){

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
                LOG(WARING)<<"ret: "<< ret <<" write into Comments fail!"<<std::endl;
                return false;
            }
            LOG(INFO)<<"write into Comments suceess"<<std::endl;
            return true;
        }



};