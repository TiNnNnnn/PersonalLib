#pragma once 
#include<cstdlib>
#include<ctime>
#include<stdint.h>
#include <sstream> 
#include<sys/time.h>
#include<iostream>
#include<unordered_map>
#include<string>
#include<vector>
#include<fstream>
#include<boost/algorithm/string.hpp>
#include<atomic>
#include<dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 *获取时间工具
*/ 
typedef struct Date
{
	int year;
	int month;
	int day;
}Date;
int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

class TimeUtil
{

  public:
    //获取时间戳
    static int64_t TimeStamp(){
      struct timeval tv;
      ::gettimeofday(&tv,NULL);
      return tv.tv_sec;
    } 
    static int64_t TimeStampMs(){
      struct timeval tv;
      ::gettimeofday(&tv,NULL);
      return tv.tv_sec * 1000 +tv.tv_usec /1000;
    }
    static bool getTime(Date* date){
      time_t now = time(0); 
      tm *ltm = localtime(&now);
      
      date->day=ltm->tm_mday;
      date->month=1+ltm->tm_mon;
      date->year=1900 + ltm->tm_year;
      return true;
    }
    static int IsLeapYear(int year)
    {
	    return ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0)) ? 1 : 0;
    }
    static void ListDate(const Date &now, int diff, Date* d1)
    {
      /* 计算diff天后的日期 */
      d1->year = now.year;
      d1->month = now.month;
      d1->day = now.day + diff;

      while (d1->day > days[d1->month - 1])
      {
          d1->day -= days[d1->month - 1] + (d1->month == 2 && IsLeapYear(d1->year));
          d1->month++;
          if (d1->month > 12)
          {
            d1->month = 1;
            d1->year++;
          }
      }
    }

};


/*
 * 打印日志工具
 * FATAL 致命 
 * ERROR 错误
 * WARNING 警告
 * INFO 提示
*/ 
enum Level
{
  INFO,WARING,ERROR,FATAL 
};
inline std::ostream& Log(Level level,const std::string& file_name , int line_num) {
  std::string prefix ="[";
  if(level == INFO){
    prefix+="I";
  }
  else if(level== WARING){
    prefix+="W";
  }
  else if(level == ERROR){
    prefix+="E";
  }
  else if(level == FATAL){
    prefix+="F";
  }
  prefix += std::to_string(TimeUtil::TimeStamp());
  prefix +=" ";
  prefix +=file_name;
  prefix +=":";
  prefix +=std::to_string(line_num);
  prefix +="]";
  std::cout<<prefix;
  return std::cout;
}

#define LOG(level) Log(level,__FILE__,__LINE__)


/*
 * 文件操作工具
 * 约定： 输入型参数 cosnt 引用
 *        输出型参数     指针
 *        输入输出型参数 引用
*/  

class FileUtil
{
  public:
    static bool Read(const std::string& file_path,std::string* content)
    {
      content->clear();
      std::ifstream file(file_path.c_str());
      if(!file.is_open()){
        LOG(ERROR)<<"open file failed!"<<std::endl;
        return false;
      }

      std::string line;
      while(std::getline(file,line)){
        *content +=line +"\n";
      }

      file.close();
      return true;
    }

    static bool Write(const std::string& file_path,const std::string& content)
    {
      std::ofstream file(file_path,std::ios::out|std::ios::app);
      if(!file.is_open()){
        LOG(ERROR)<<"file write or create fail!"<<std::endl;
        return false;
      }
      LOG(INFO)<<"file write or create sucess!"<<std::endl;
      file.write(content.c_str(),content.size());
      file.close();
      return true;
    }

    //读取图片文件
    static bool Image_Read(const std::string& file_path,std::string* content){
       // 1. 打开图片文件
        std::ifstream is(file_path, std::ifstream::in | std::ios::binary);
        if(!is.is_open()){
          return false;
        }
        // 2. 计算图片长度
        is.seekg(0, is.end);  //将文件流指针定位到流的末尾
        int length = is.tellg();
        is.seekg(0, is.beg);  //将文件流指针重新定位到流的开始
        // 3. 创建内存缓存区
        char * buffer = new char[length];
        // 4. 读取图片
        is.read(buffer, length);
        // 到此，图片已经成功的被读取到内存（buffer）中
        *content=buffer;
        delete [] buffer;
        return true;
    }
   
};

class GetFile{
  public:
      //生成新的书籍附加信息文件名字
      static std::string BookTxtPath(const std::string& name,const std::string& id){
          return "./book_data/" + id +"/"+name + ".txt";
      }
      //生成新的用户附加信息文件名字
      static std::string UserTxtPath(const std::string& name,const std::string& id){
          return "./user_data/" + id +"/"+name + ".txt";
      }
      

      static bool GetNewFileForBook(const std::string path,const std::string& desc, const std::string& bid){
          CreateDir(path,bid);
          std::string filename = WriteTmpFileForBook(desc,bid);
          LOG(INFO)<<"new book "<<bid<<"'s desc-filename:"<<filename<<std::endl; 
          return true;
      }

      static bool GetNewFileForUser(const std::string path,const std::string& desc, const std::string& uid){
          CreateDir(path,uid);
          std::string filename = WriteTmpFileForUser(desc,uid);
          LOG(INFO)<<"new user "<<uid<<"'s desc-filename:"<<filename<<std::endl; 
          return true;
      }
      

  private:
      // 1. 创建文件把描述写到文件中
      // 2. 将新书信息写入 入口文件
      static std::string WriteTmpFileForBook(const std::string& desc,const std::string& bid){
            std::string header = "说说你的读后感吧！";
            
            std::string filename;
            FileUtil::Write(filename = BookTxtPath("desc",bid),desc);
            FileUtil::Write(BookTxtPath("header",bid),header);

            std::string door = '\n'+bid+'\t'+"./book_data/"+bid;
            FileUtil::Write("./book_data/book_config.cfg",door);

            return filename;
      }

       // 1. 创建文件把描述写到文件中
      // 2. 将新用户信息写入 入口文件
      static std::string WriteTmpFileForUser(const std::string& desc,const std::string& uid){
            std::string header = "说说自己喜欢的书，想看的书吧.....";
            
            std::string filename;
            FileUtil::Write(filename = UserTxtPath("desc",uid),desc);
            FileUtil::Write(UserTxtPath("header",uid),header);

            std::string door ='\n'+uid+'\t'+"./user_data/"+uid;
            FileUtil::Write("./user_data/user_config.cfg",door);

            return filename;
      }

      //创建文件夹
      static std::string CreateDir(const std::string path,const std::string& uid){
           std::string dir_path = path+std::string(uid);
           if(nullptr == opendir(dir_path.c_str())){
              if(-1==::mkdir(dir_path.c_str(),0777)){
                LOG(ERROR)<<"dir: "<<dir_path<<" create fail"<<std::endl;
                return nullptr;
              }
              LOG(INFO)<<"dir: "<<dir_path<<" create success"<<std::endl;
              return dir_path;
           }
           LOG(WARING)<<"dir: "<<dir_path<<" has exist!"<<std::endl;
           return dir_path;
           
      }
};

/*
/ url /body 解析模块

  一些切分方法：
    1.strtok
    2.stringstream
    3.boost split 函数
*/

class StringUtil{
public:
  static void Split(const std::string& input,const std::string& split_char,
        std::vector<std::string>* output){

          boost::split(*output,input,boost::is_any_of(split_char),
                boost::token_compress_off);
   }

};



class UrlUtil{
public:
  static void ParseBody(const std::string& body,
      std::unordered_map<std::string,std::string>* params){
        //1.对body字符串切分为键值对 
        // a) 先按照 & 符号切分
        // b) 再按照 = 切分
        std::vector<std::string> kvs;
        StringUtil::Split(body,"&",&kvs);
        for(size_t i =0;i<kvs.size();++i){
          std::vector<std::string>kv;
          StringUtil::Split(kvs[i],"=",&kv);
          if(kv.size()!=2){
            continue;
          }
          (*params)[kv[0]]=UrlDecode(kv[1]);
        }

        //2.对这里的键值对进行 urldecode
  }

  static unsigned char ToHex(unsigned char x) 
  { 
      return  x > 9 ? x + 55 : x + 48; 
  }
  
  static unsigned char FromHex(unsigned char x) 
  { 
      unsigned char y;
      if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
      else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
      else if (x >= '0' && x <= '9') y = x - '0';
      else assert(0);
      return y;
  }
  
  static std::string UrlEncode(const std::string& str)
  {
      std::string strTemp = "";
      size_t length = str.length();
      for (size_t i = 0; i < length; i++)
      {
          if (isalnum((unsigned char)str[i]) || 
              (str[i] == '-') ||
              (str[i] == '_') || 
              (str[i] == '.') || 
              (str[i] == '~'))
              strTemp += str[i];
          else if (str[i] == ' ')
              strTemp += "+";
          else
          {
              strTemp += '%';
              strTemp += ToHex((unsigned char)str[i] >> 4);
              strTemp += ToHex((unsigned char)str[i] % 16);
          }
      }
      return strTemp;
  }
  
  static std::string UrlDecode(const std::string& str)
  {
      std::string strTemp = "";
      size_t length = str.length();
      for (size_t i = 0; i < length; i++)
      {
          if (str[i] == '+') strTemp += ' ';
          else if (str[i] == '%')
          {
              assert(i + 2 < length);
              unsigned char high = FromHex((unsigned char)str[++i]);
              unsigned char low = FromHex((unsigned char)str[++i]);
              strTemp += high*16 + low;
          }
          else strTemp += str[i];
      }
      return strTemp;
  }

};




