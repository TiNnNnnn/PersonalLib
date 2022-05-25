#pragma once
#include<iostream>
#include<vector>
#include<string>
#include"user_model.hpp"
#include<ctemplate/template.h>


class UserView{
    public:
    //总用户 列表 
        static void RenderAllUsers(const std::vector<User>& all_users,std::string* html){
                //将所有题目数据转化为题目列表
                //通过网页模板的方式解决 手动拼接
            
            ctemplate::TemplateDictionary dict("all_users");
            for(const auto& user :all_users){
                ctemplate::TemplateDictionary* table_dict=dict.AddSectionDictionary("user");
                table_dict->SetValue("id", user.uid); 
                table_dict->SetValue("name",user.uname);
                table_dict->SetValue("phone",user.phone);
                table_dict->SetValue("email",user.email);
                table_dict->SetValue("address",user.adress);
            }
            
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/all_users.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }

        //获取书籍上传页
        static void RenderUpload(const User& user,std::string* html){
            
            ctemplate::TemplateDictionary dict("user");
            dict.SetValue("id",user.uid);
            dict.SetValue("name",user.uname);
            
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/upload.html", ctemplate::DO_NOT_STRIP
            );

            tpl->Expand(html, &dict);
        }

        //用户主页
        static void RenderUserPersonal(const User& user,const std::vector<Book>& my_books,const std::vector<Book>& my_borrow,const std::vector<Book>& my_lend,std::string* html){
            
            ctemplate::TemplateDictionary dict("my_books");
            dict.SetValue("uid", user.uid); 
            dict.SetValue("uname",user.uname);
            dict.SetValue("phone",user.phone);
            dict.SetValue("email",user.email);
            dict.SetValue("address",user.adress);
            dict.SetValue("img",user.img);
            dict.SetValue("desc",user.desc);

            for(const auto& book :my_books){
                ctemplate::TemplateDictionary* table_dict=dict.AddSectionDictionary("book");
                table_dict->SetValue("id",book.id); 
                table_dict->SetValue("name",book.name);
                table_dict->SetValue("author",book.author);
                table_dict->SetValue("isbn",book.isbn);
                table_dict->SetValue("owner",book.owner);
            } 
            //我的借出
            for(const auto& book :my_lend){
                ctemplate::TemplateDictionary* table_dict=dict.AddSectionDictionary("lbook");
                table_dict->SetValue("lid",book.id); 
                table_dict->SetValue("lname",book.name);
                table_dict->SetValue("lauthor",book.author);
                table_dict->SetValue("lisbn",book.isbn);
                table_dict->SetValue("lowner",book.owner);
                table_dict->SetValue("lborrow_time",book.lend_time);
                table_dict->SetValue("lback_time",book.back_time);
            }   

            //我的借入
            for(const auto& book :my_borrow){
                ctemplate::TemplateDictionary* table_dict=dict.AddSectionDictionary("bbook");
                table_dict->SetValue("bid",book.id); 
                table_dict->SetValue("bname",book.name);
                table_dict->SetValue("bauthor",book.author);
                table_dict->SetValue("bisbn",book.isbn);
                table_dict->SetValue("bowner",book.owner);
                table_dict->SetValue("bborrow_time",book.lend_time);
                table_dict->SetValue("bback_time",book.back_time);
                
            }
            
                  
                     
        
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/personal.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }

        //获取用户登录页  （当用户注册之后跳转）  
        static void RenderEnroll(const User& user,std::string* html){
            
            ctemplate::TemplateDictionary dict("user");
            dict.SetValue("id",user.uid);
            dict.SetValue("name",user.uname);
            
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/login.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }

        //获取用户主页  （当用户登录之后跳转）
        static void RenderLogin(const User& user,const std::vector<Book>& all_books, std::string* html){
            
            ctemplate::TemplateDictionary dict("all_books");
            dict.SetValue("uid",user.uid);
            dict.SetValue("uname",user.uname);

            for(const auto& book :all_books){
                ctemplate::TemplateDictionary* table_dict=dict.AddSectionDictionary("book");
                table_dict->SetValue("id",book.id); 
                table_dict->SetValue("name",book.name);
                table_dict->SetValue("author",book.author);
                table_dict->SetValue("isbn",book.isbn);
                table_dict->SetValue("owner",book.owner);
            }  
            
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/index.html", ctemplate::DO_NOT_STRIP// 暂时跳转所有用户页
            );

            tpl->Expand(html, &dict);
        }

        //搜索结果页
        static void RenderSearchResult(const User& user,const std::vector<Book>& search_books, std::string* html){
            
            ctemplate::TemplateDictionary dict("search_books");
            dict.SetValue("uid",user.uid);
            dict.SetValue("uname",user.uname);

            for(const auto& book :search_books){
                ctemplate::TemplateDictionary* table_dict=dict.AddSectionDictionary("book");
                table_dict->SetValue("id",book.id); 
                table_dict->SetValue("name",book.name);
                table_dict->SetValue("author",book.author);
                table_dict->SetValue("isbn",book.isbn);
                table_dict->SetValue("owner",book.owner);
            }  
            
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/findresult.html", ctemplate::DO_NOT_STRIP// 暂时跳转所有用户页
            );

            tpl->Expand(html, &dict);
        }


        

};