#include<iostream>
#include"httplib.h"
#include"util.hpp"
#include<string>
#include"book_model.hpp"
#include"user_model.hpp"
#include"comm_model.hpp"
#include"book_view.hpp"
#include"user_view.hpp"
#include "json/json.h"

int main()
{
    BookModel model;
    model.Load();

    UserModel umodel;
    umodel.Load();

    CommModel cmodel;
    cmodel.Load();

    using namespace httplib;
    Server server;

    //获取所有书籍
    server.Get("/all_books",[&model](const Request& req,Response& resp){
        (void)req;
        //数据来源于 Model 对象
        std::vector<Book>all_books;
        model.GetAllBooks(&all_books);

        //将数据 渲染为 html
        std::string html;
        BookView::RenderAllBooks(all_books,&html);
        resp.set_content(html,"text/html"); //写入响应报头
    });

    //按照书籍id 获取书籍信息
    server.Get(R"(/book/(\d+)/(\d+))",[&model,&umodel,&cmodel](const Request& req,Response& resp){
        Book book;
        User user;
        std::vector<Comm>book_comm;

        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<req.matches[2]<<std::endl;
        model.GetBook(req.matches[1].str(),&book);
        umodel.GetUser(req.matches[2].str(),&user);
        cmodel.GetCommsByBid(book.id,&book_comm);

        std::string html;
        BookView::RenderBook(user,book,book_comm,&html);
        resp.set_content(html,"text/html");
    });
    
    //按照用户名拿取 用户书库
    server.Get(R"(/book/(\S+))",[&model](const Request& req,Response& resp){
        
        std::vector<Book>my_books;
        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<std::endl;
        model.GetMyBooksByUname(req.matches[1].str(),&my_books);
        std::string html;
        BookView::RenderMyBooks(my_books,&html);
        resp.set_content(html,"text/html");

        for(auto e:req.matches){
            std::cout<<e<<" ";
        }
    });

    //提交书籍申请
    server.Post(R"(/uploadbook/(\d+))",[&model,&umodel](const Request& req,Response& resp){
        
        //MultipartFiles ret = req.files;
        std::cout<<req.body<<std::endl;

        //1. 解析body，获取到用户提交的数据
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);

        const std::string& bname = body_kv["bname"];
        const std::string& author = body_kv["author"];
        const std::string& isbn = body_kv["isbn"];
        const std::string& lendtime = body_kv["lendtime"];
        const std::string& file=body_kv["files"];
        std::string& book_desc = body_kv["desc"];


        if(!bname.size() ||!author.size() || !isbn.size() || !lendtime.size()){
            LOG(ERROR)<<"the data about book has error"<<std::endl;
            return 9;
        }
        if(!book_desc.size()){
            book_desc = "这本书很神秘，没有介绍！";
        }
        

        //2.构建插入语句
        std::vector<Book>all_books;
        model.GetAllBooks(&all_books);
        int bid = all_books.size()+1; //生成用户id
        std::string bbid = std::to_string(bid);

        LOG(INFO)<<"debug"<<req.matches[0]<<" "<<req.matches[1]<<std::endl;

        //3.查找用户uid
        User user;
        if(!umodel.GetUser(req.matches[1].str(),&user)){
            LOG(WARING)<<"this user is not exist!"<<std::endl;
            return 1;
        }

        const std::string& uname = user.uname;
        
        std::string sql="INSERT INTO BookBase VALUES (";
        sql+= bbid+",'";
        sql+= bname + "','";
        sql+= author +"','";
        sql+= isbn +"','";
        sql+= uname +"','";//owner
        sql+= "NULL','"; //boworrer
        sql+= "0','";//islend
        sql+= "0','"  ;//waitnum
        sql+= req.matches[1].str() +"','"; //从用户表里面查找uid
        sql+= "NULL','"; //lendtime
        sql+= "NULL');"; 

        //3.查找书籍是否已经存在
        if(model.GetBookByISBN(isbn,new Book)){
            LOG(WARING)<<"this book has exist!"<<std::endl;
            return 2;
        }

        else{
            //插入
            if(!model.InsertBook(sql)){
                LOG(ERROR)<<"insert BookBase error"<<std::endl;
                return 3;
            }
        }

        //将描述信息写入文本库
        GetFile::GetNewFileForBook("./book_data/",book_desc,std::to_string(bid));
        model.Load();
        //3. 构造 JSON 结构的参数
        Json::Value req_json;
        
        //5.根据插入结果或出错原因建立响应
        std::string html;
        //BookView::RenderResult(user,html)
        resp.set_content(html,"text/html");
        return 0;
    });

    //获取所有用户
    server.Get("/all_users",[&umodel](const Request& req,Response& resp){
        (void)req;
        //数据来源于 Model 对象
        std::vector<User>all_users;
        umodel.GetAllUsers(&all_users);

        //将数据 渲染为 html
        std::string html;
        UserView::RenderAllUsers(all_users,&html);
        resp.set_content(html,"text/html"); //写入响应报头
    });

    //获取用户主页 （信息，个人书库，个人借出，个人借阅）
    server.Get(R"(/personal/(\d+))",[&model,&umodel](const Request& req,Response& resp){
        User user;
        std::vector<Book>my_books;
        std::vector<Book>my_borrow;
        std::vector<Book>my_lend;
        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<std::endl;
        //获取用户信息 
        umodel.GetUser(req.matches[1].str(),&user);
        //获取用户书库  
        model.GetMyBooksById(req.matches[1].str(),&my_books);
        //获取用户已借
        model.GetMyBorrow(user.uname,&my_borrow);
        //获取用户借出
        model.GetMyLend(user.uid,&my_lend);

        std::string html;
        UserView::RenderUserPersonal(user,my_books,my_borrow,my_lend,&html);
        resp.set_content(html,"text/html");
    });

    //获取用户上传页面
    server.Get(R"(/upload/(\d+))",[&umodel](const Request& req,Response& resp){
        User user;
        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<std::endl;
        //获取用户信息 
        umodel.GetUser(req.matches[1].str(),&user);
        std::string html;
        UserView::RenderUpload(user,&html);
        resp.set_content(html,"text/html");
    });

    //接受用户注册数据
    server.Post(R"(/enroll)",[&model,&umodel](const Request& req,Response& resp){

        //1. 解析body，获取到用户提交的数据
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);

        const std::string& uname = body_kv["uname"];
        const std::string& phone = body_kv["phone"];
        const std::string& email = body_kv["email"];
        const std::string& address = body_kv["address"];
        const std::string& passwd = body_kv["passwd"];
        std::string& user_desc = body_kv["desc"];

        if(!uname.size()||phone.size()<8||email.size()<8||address.size()<2||passwd.size()<4){
            LOG(WARING)<<"User information submission is incomplete or in a error format"<<std::endl;
            return 5;
        }
        if(user_desc.size()==0) user_desc="这个用户很神秘，啥也没写\n"; 

        //2.构建插入语句
        std::vector<User>all_users;
        umodel.GetAllUsers(&all_users);
        int uid = all_users.size()+1; //获取用户id
        std::string uuid = std::to_string(uid);

        LOG(INFO)<<"debug"<<req.matches[0]<<" "<<req.matches[1]<<std::endl;

        //3.查找用户uid
        User user;
        if(umodel.GetUser(uuid,&user)){
            LOG(WARING)<<"this user has exist!"<<std::endl;
            return 1;
        }
        
        std::string sql="INSERT INTO user VALUES (";
        sql+= uuid+",'";
        sql+= uname + "','";
        sql+= phone +"','";
        sql+= email +"','";
        sql+= address +"','";
        sql+= passwd +"');";
    
        
        //插入
        if(!umodel.InsertBook(sql)){
            LOG(ERROR)<<"insert user error"<<std::endl;
            return 2;
        }
        LOG(INFO)<<"insert user sucess"<<std::endl;
        

        //将描述信息写入文本库
        GetFile::GetNewFileForUser("./user_data/",user_desc,std::to_string(uid));
        umodel.Load();

        //3. 构造 JSON 结构的参数
        Json::Value req_json;
        
        //5.根据插入结果或出错原因建立响应
        std::string html;
        UserView::RenderEnroll(user,&html);
        resp.set_content(html,"text/html");
        return 0;
        
    });

    //接受用户登录数据
    server.Post(R"(/login)",[&model,&umodel](const Request& req,Response& resp){

        //1. 解析body，获取到用户提交的数据
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);

        std::vector<Book>all_books;
        model.GetAllBooks(&all_books);

        const std::string& uname = body_kv["uname"];
        const std::string& passwd = body_kv["passwd"];

        LOG(INFO)<<uname<<" "<<passwd<<std::endl;

        //3.验证用户是否存在
        User user;
        if(!umodel.GetUserVerify(uname,passwd,&user)){
            LOG(WARING)<<"this user isn't exist!"<<std::endl;
            return 1;
        }

        //4.返回网页
        std::string html;
        UserView::RenderLogin(user,all_books,&html);
        resp.set_content(html,"text/html");

        return 0;
    });

   
    //按照搜索内容 返回结果
    server.Post(R"(/search/(\d+))",[&model,&umodel](const Request& req,Response& resp){
        //1. 解析body，获取到用户提交的数据
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);

        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<std::endl;

        User user;
        umodel.GetUser(req.matches[1].str(),&user);

        const std::string& sfrom = body_kv["sfrom"];  //搜索方式
        const std::string& search = body_kv["search"];//搜索内容


        LOG(INFO)<<sfrom<<" "<<search<<std::endl;

        //3.获取搜索结果
        std::vector<Book>search_books;
        model.GetUserSearchResult(sfrom,search,&search_books);
        
        //4.返回搜索结果网页
        std::string html;
        UserView::RenderSearchResult(user,search_books,&html);
        resp.set_content(html,"text/html");

        return 0;
    });

    //用户进入借阅界面
    server.Get(R"(/borrow/(\d+)/(\d+))",[&model,&umodel](const Request& req,Response& resp){
       
        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<req.matches[2]<<std::endl;

        //获取当前时间
        Date now;
        TimeUtil::getTime(&now);
        std::string nowtime = std::to_string(now.year)+"-"+ std::to_string(now.month)+"-"+std::to_string(now.day);//将时间转化为字符串格式
        //2.获取将借阅用户信息 
        User user;
        umodel.GetUser(req.matches[2].str(),&user);
        //3.获取将被借阅书籍信息
        Book book;
        model.GetBook(req.matches[1].str(),&book);
        book.lend_time=nowtime;

        std::string html;
        BookView::RenderBorrow(user,book,&html);
        resp.set_content(html,"text/html");
    });


    //接收用户借阅数据，更新数据库
    server.Post(R"(/update/(\d+)/(\d+))",[&model,&umodel](const Request& req,Response& resp){
        //1. 解析body，获取到用户提交的数据
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);
        const std::string& lendtime = body_kv["lendtime"];  //借阅时间
        if(std::stoi(lendtime)>90){
            LOG(WARING)<<"you can't borrowed more than 3 months!"<<std::endl;
            return 1; 
        }

        //获取当前时间
        Date now;
        TimeUtil::getTime(&now);
        std::string nowtime = std::to_string(now.year)+"-"+ std::to_string(now.month)+"-"+std::to_string(now.day);//将时间转化为字符串格式
        //获取应还时间
        Date back;
        TimeUtil::ListDate(now,std::stoi(lendtime),&back);
        std::string backtime = std::to_string(back.year)+"-"+ std::to_string(back.month)+"-"+std::to_string(back.day);//将时间转化为字符串格式

        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<req.matches[2]<<std::endl;

        //2.获取将借阅用户信息
        User user;
        umodel.GetUser(req.matches[2].str(),&user);
        //3.获取将被借阅书籍信息
        Book book;
        model.GetBook(req.matches[1].str(),&book);

        //4.验证是否可借
        if(book.islend=="1"){
            LOG(WARING)<<"the book has been brrowered"<<std::endl;
            return 2;
        }
        //5.验证借书人不是本人
        if(book.owner==user.uname){
            LOG(WARING)<<"you can't borrow yourself!"<<std::endl;
            return 3;
        }
        //5.更新借阅人，借阅时间，应还时间,排队人数
        std::string sql ="update BookBase set boworrer = '"+user.uname+"', ";
        sql += "islend = '1', ";
        sql += "waitnum = '"+ std::to_string(std::stoi(book.num)+1)+"', ";
        sql += "lend_time = '"+ nowtime +"', ";
        sql += "borrow_time = '" + backtime +"' ";    
        sql += "where bid = '"+req.matches[1].str()+"';";

        model.UpDateBook(sql);
        model.Load();
        
        //4.返回搜索结果网页
        //std::string html;
        //UserView::RenderBorrowResult(user,book,&html);
        //resp.set_content(html,"text/html");

        return 0;
    });


    //进入归还界面 back.html
    server.Get(R"(/back/(\d+)/(\d+))",[&model,&umodel](const Request& req,Response& resp){
       
        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<req.matches[2]<<std::endl;

        //获取当前时间
        Date now;
        TimeUtil::getTime(&now);
        std::string nowtime = std::to_string(now.year)+"-"+ std::to_string(now.month)+"-"+std::to_string(now.day);//将时间转化为字符串格式
        //2.获取将还书用户信息 
        User user;
        umodel.GetUser(req.matches[2].str(),&user);
        //3.获取将被还书书籍信息
        Book book;
        model.GetBook(req.matches[1].str(),&book);
        book.lend_time=nowtime;

        std::string html;
        BookView::RenderBack(user,book,&html);
        resp.set_content(html,"text/html");
    });

     //处理用户归还请求，修改数据库
    server.Post(R"(/updateback/(\d+)/(\d+))",[&model,&umodel,&cmodel](const Request& req,Response& resp){
        //1. 解析body，获取到用户提交的评论
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);
        const std::string& comments = body_kv["comments"];  //评论
        
        //获取当前时间
        Date now;
        TimeUtil::getTime(&now);
        std::string nowtime = std::to_string(now.year)+"-"+ std::to_string(now.month)+"-"+std::to_string(now.day);//将时间转化为字符串格式

        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<req.matches[2]<<std::endl;

        //2.获取还书用户信息
        User user;
        umodel.GetUser(req.matches[2].str(),&user);
        //3.获取被还书籍信息
        Book book;
        model.GetBook(req.matches[1].str(),&book);

        //5.插入评论表  
        std::string sql1="INSERT INTO Comments VALUES (";
        sql1+= "NULL,";
        sql1+= std::to_string(std::stoi(book.id)) + ",";
        sql1+= std::to_string(std::stoi(user.uid)) +",'";
        sql1+= comments +"','";
        sql1+= user.uname +"','";
        sql1+= book.owner +"','";
        sql1+= nowtime + "');";
        cmodel.InsertComments(sql1);

        cmodel.Load();

        //获取应还时间
        std::string backtime = book.back_time;

        //5.更新借阅人，借阅时间，应还时间,排队人数
        std::string sql2 ="update BookBase set boworrer = 'NULL', ";
        sql2 += "islend = '0', ";
        sql2 += "waitnum = '"+ std::to_string(std::stoi(book.num)-1)+"', ";
        sql2 += "lend_time = 'NULL', ";
        sql2 += "borrow_time = 'NULL' ";    
        sql2 += "where bid = '"+req.matches[1].str()+"';";

        model.UpDateBook(sql2);
        model.Load();
        
        //4.返回搜索结果网页
        //std::string html;
        //UserView::RenderBorrowResult(user,book,&html);
        //resp.set_content(html,"text/html");
        return 0;
    });

    //接受用户评论信息
    server.Post(R"(/comment/(\d+)/(\d+))",[&model,&umodel,&cmodel](const Request& req,Response& resp){
        //1. 解析body，获取到用户提交的评论
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);
        const std::string& comments = body_kv["comments"];  //评论

        if(comments.size()==0){
            LOG(WARING)<<"comments is empty!"<<std::endl;
            return 1;
        }
        //获取当前评论时间
        Date now;
        TimeUtil::getTime(&now);
        std::string nowtime = std::to_string(now.year)+"-"+ std::to_string(now.month)+"-"+std::to_string(now.day);//将时间转化为字符串格式

        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<req.matches[2]<<std::endl;

        //2.获取评论用户信息
        User user;
        umodel.GetUser(req.matches[2].str(),&user);
        //3.获取被评价书籍信息
        Book book;
        model.GetBook(req.matches[1].str(),&book);

        //5.插入评论表  
        std::string sql="INSERT INTO Comments VALUES (";
        sql+= "NULL,";
        sql+= std::to_string(std::stoi(book.id)) + ",";
        sql+= std::to_string(std::stoi(user.uid)) +",'";
        sql+= comments +"','";
        sql+= user.uname +"','";
        sql+= book.owner +"','";
        sql+= nowtime + "');";

        cmodel.InsertComments(sql);
        cmodel.Load();
        
        //跳转回书籍首页？
        std::string html = "评论成功";
        //std::string html;
        //UserView::RenderBorrowResult(user,book,&html);
        resp.set_content(html,"text/html");
        return 0;
    });


    //接受用户评论评论信息
    server.Post(R"(/comment/(\d+)/(\d+)/(\S+))",[&model,&umodel,&cmodel](const Request& req,Response& resp){
        //1. 解析body，获取到用户提交的评论
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);
        const std::string& comments = body_kv["comments"];  //评论

        if(comments.size()==0){
            LOG(WARING)<<"comments is empty!"<<std::endl;
            return 1;
        }
        //获取当前评论时间
        Date now;
        TimeUtil::getTime(&now);
        std::string nowtime = std::to_string(now.year)+"-"+ std::to_string(now.month)+"-"+std::to_string(now.day);//将时间转化为字符串格式

        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<req.matches[2]<<" "<< req.matches[3]<<std::endl;

        //2.获取评论用户信息

        User user;  //评论用户
        User user2; //被评论用户

        umodel.GetUser(req.matches[2].str(),&user);
        //3.获取被评论用户信息
        umodel.GetUserByName(req.matches[3].str(),&user2);
        //3.获取被评价书籍信息
        Book book;
        model.GetBook(req.matches[1].str(),&book);

        //5.插入评论表  
        std::string sql="INSERT INTO Comments VALUES (";
        sql+= "NULL,";
        sql+= std::to_string(std::stoi(book.id)) + ",";
        sql+= std::to_string(std::stoi(user.uid)) +",'";
        sql+= comments +"','";
        sql+= user.uname +"','";
        sql+= user2.uname +"','";
        sql+= nowtime + "');";

        cmodel.InsertComments(sql);
        cmodel.Load();
        
        //跳转回书籍首页？

        //std::string html;
        //UserView::RenderBorrowResult(user,book,&html);
        //resp.set_content(html,"text/html");
        return 0;
    });

    //消息窗口
    server.Post(R"(/communicate/(\d+)/(\d+))",[&model,&umodel,&cmodel](const Request& req,Response& resp){
        //1. 解析body，获取用户发过来的消息
        std::unordered_map<std::string,std::string>body_kv;
        UrlUtil::ParseBody(req.body,&body_kv);
        const std::string& comments = body_kv["comments"];  //评论

        if(comments.size()==0){
            LOG(WARING)<<"comments is empty!"<<std::endl;
            return 1;
        }
        //获取当前评论时间
        Date now;
        TimeUtil::getTime(&now);
        std::string nowtime = std::to_string(now.year)+"-"+ std::to_string(now.month)+"-"+std::to_string(now.day);//将时间转化为字符串格式

        LOG(INFO)<<req.matches[0]<<" "<<req.matches[1]<<" "<<req.matches[2]<<" "<< req.matches[3]<<std::endl;

        //2.获取评论用户信息

        User user;  //评论用户
        User user2; //被评论用户

        umodel.GetUser(req.matches[2].str(),&user);
        //3.获取被评论用户信息
        umodel.GetUserByName(req.matches[3].str(),&user2);
        //3.获取被评价书籍信息
        Book book;
        model.GetBook(req.matches[1].str(),&book);
        
        //5.插入评论表  
        std::string sql="INSERT INTO Comments VALUES (";
        sql+= "NULL,";
        sql+= std::to_string(std::stoi(book.id)) + ",";
        sql+= std::to_string(std::stoi(user.uid)) +",'";
        sql+= comments +"','";
        sql+= user.uname +"','";
        sql+= user2.uname +"','";
        sql+= nowtime + "');";

        cmodel.InsertComments(sql);
        cmodel.Load();
        
        //跳转回书籍首页？

        //std::string html;
        //UserView::RenderBorrowResult(user,book,&html);
        //resp.set_content(html,"text/html");
        return 0;
    });

   

    server.set_base_dir("./template");
    server.listen("0.0.0.0",9099);

    return 0;

}
