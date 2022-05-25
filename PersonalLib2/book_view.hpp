#pragma once
#include<iostream>
#include<vector>
#include<string>
#include"comm_model.hpp"
#include"book_model.hpp"
#include<ctemplate/template.h>


class BookView{
    public:
        //网页渲染：根据数据，生成 html

        //总书库 列表 
        static void RenderAllBooks(const std::vector<Book>& all_books,std::string* html){
                //将所有题目数据转化为题目列表
                //通过网页模板的方式解决 手动拼接
            ctemplate::TemplateDictionary dict("all_books");
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
                "./template/all_books.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }
        //书籍详情页
        static void RenderBook(const User& user,const Book& book,std::vector<Comm>book_comms, std::string* html){
            
            ctemplate::TemplateDictionary dict("book");

            dict.SetValue("uid",user.uid);
            dict.SetValue("id",book.id); 
            dict.SetValue("name",book.name);
            dict.SetValue("author",book.author);
            dict.SetValue("isbn",book.isbn);
            dict.SetValue("owern",book.owner);
            if(book.islend=="0"){
                dict.SetValue("islend","可借阅");
                dict.SetValue("color","228B22");
            }else{
                dict.SetValue("islend","已借出");
                dict.SetValue("color","FF7F50");
            }
            dict.SetValue("borrower",book.borrower);
            dict.SetValue("img",book.img);
            dict.SetValue("desc",book.desc);

            for(const auto& comms :book_comms){
                ctemplate::TemplateDictionary* table_dict=dict.AddSectionDictionary("comm");
                table_dict->SetValue("cid",comms.cid);
                table_dict->SetValue("content",comms.content);
                table_dict->SetValue("responser",comms.Responser);

                if(comms.Respondent == book.owner)table_dict->SetValue("respondent","");
                else table_dict->SetValue("respondent"," 回复 "+comms.Respondent);

                table_dict->SetValue("ctime",comms.ctime);
                table_dict->SetValue("color2","#20B2AA");

                //当 书籍评论者自己评论时
                //if(comms.Responser == book.owner)table_dict->SetValue("","(书主)");
            }
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/book.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }

        //我的书库
        static void RenderMyBooks(const std::vector<Book>& my_books,std::string* html){
                //将所有题目数据转化为题目列表
                //通过网页模板的方式解决 手动拼接
            ctemplate::TemplateDictionary dict("my_books");
            for(const auto& book :my_books){
                ctemplate::TemplateDictionary* table_dict=
                    dict.AddSectionDictionary("book");
                table_dict->SetValue("id",book.id); 
                table_dict->SetValue("name",book.name);
                table_dict->SetValue("author",book.author);
                table_dict->SetValue("isbn",book.isbn);
                //table_dict->SetValue("owner",book.owner);
            }
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/my_books.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }
        
        //我借出的书
        static void RenderMylend(const std::vector<Book>& my_books,std::string* html){
                //将所有题目数据转化为题目列表
                //通过网页模板的方式解决 手动拼接
            ctemplate::TemplateDictionary dict("my_books");
            for(const auto& book :my_books){
                ctemplate::TemplateDictionary* table_dict=
                    dict.AddSectionDictionary("book");
                table_dict->SetValue("id",book.id); 
                table_dict->SetValue("name",book.name);
                table_dict->SetValue("author",book.author);
                table_dict->SetValue("isbn",book.isbn);
                //table_dict->SetValue("owner",book.owner);
            }
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/my_books.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }

        //借书页面
        static void RenderBorrow(const User& user,const Book& book,std::string* html){
            
            ctemplate::TemplateDictionary dict("book");
            dict.SetValue("uid",user.uid);
            dict.SetValue("uname",user.uname);
            dict.SetValue("id",book.id); 
            dict.SetValue("name",book.name);
            dict.SetValue("now_time",book.lend_time);
            
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/borrow.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }

        //还书页面
        static void RenderBack(const User& user,const Book& book,std::string* html){
            
            ctemplate::TemplateDictionary dict("book");
            dict.SetValue("uid",user.uid);
            dict.SetValue("uname",user.uname);
            dict.SetValue("id",book.id); 
            dict.SetValue("bname",book.name);
            dict.SetValue("now_time",book.lend_time);
            dict.SetValue("back_time",book.back_time);
            
            ctemplate::Template* tpl;
            tpl = ctemplate::Template::GetTemplate(
                "./template/back.html", ctemplate::DO_NOT_STRIP
            );
            tpl->Expand(html, &dict);
        }
};
