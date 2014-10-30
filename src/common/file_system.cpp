#include "common/file_system.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <unistd.h>

#include "common/url.h"
#include "common/logger.h"
#include "common/convert2string.h"

namespace
{
    bool create_directory_impl(const char* path) WARN_UNUSED_RESULT
    {
#ifdef OS_WIN
        bool result = mkdir(path) != ERROR_RESULT_VALUE;
#else
        bool result = mkdir(path, S_IRWXU|S_IRWXG|S_IRWXO) != ERROR_RESULT_VALUE;
#endif
        if(!result){
            common::DEBUG_MSG_PERROR("mkdir");
        }

        return result;
    }
}

namespace common
{
    namespace file_system
    {
        std::string prepare_path(std::string path)
        {
            if(path.empty()){
                return std::string();
            }

             if(path[0] == '~'){
             #ifdef OS_POSIX
                 char* home = getenv("HOME");
             #else
                 char* home = getenv("USERPROFILE");
             #endif
                 if(home){
                     std::string tmp = path;
                     path = home;
                     path += tmp.substr(1);
                 }
             }

             std::replace(path.begin(), path.end(), '\\', get_separator<char>());
             return path;
        }

        tribool is_directory(const std::string &path)
        {
            if(path.empty()){
                return INDETERMINATE;
            }

            struct stat filestat;
            std::string p_path = prepare_path(path);
            if (::stat(p_path.c_str(), &filestat ) != ERROR_RESULT_VALUE){
                return S_ISDIR(filestat.st_mode) ? SUCCESS : FAIL;
            }
            else{
                DEBUG_MSG_PERROR("stat");
                return INDETERMINATE;
            }
        }

        bool clear_file_by_descriptor(int fd_desc)
        {
            if(fd_desc == INVALID_DESCRIPTOR){
                return false;
            }

            bool result = ftruncate(fd_desc,0)!=ERROR_RESULT_VALUE;
            if(!result){
                DEBUG_MSG_PERROR("ftruncate");
            }
            return result;
        }

#ifdef OS_POSIX
        bool create_node(const std::string &path)
        {
            return create_node(path, S_IRWXU|S_IRWXG|S_IRWXO);
        }

        bool create_node(const std::string &path, size_t permissions)
        {
            if(path.empty()){
                return false;
            }

            bool result = mknod(path.c_str(), permissions, 0) != ERROR_RESULT_VALUE;
            if(!result){
                DEBUG_MSG_PERROR("mknod");
            }

            return result;
        }
#else
        bool create_node(const std::string &path)
        {
            if(path.empty()){
                return false;
            }

            #pragma message("IMPLEMENT PLZ")
            return false;
        }
#endif
        bool create_directory(const std::string& path, bool isRecursive)
        {
            if(path.empty()){
                return false;
            }

            std::string prPath = prepare_path(path);
            if(prPath[prPath.length() - 1] == get_separator<char>()){
                prPath[prPath.length() - 1] = 0;
            }

            const char* prPathPtr = prPath.c_str();

            if(isRecursive){
                char *p = NULL;
#ifdef OS_WIN
                uint8_t shift = 3;
#else
                uint8_t shift = 1;
#endif
                for(p = const_cast<char*>(prPathPtr + shift); *p; p++ ){
                    if(*p == get_separator<char>()){
                        *p = 0;
                        const char *path = prPathPtr;

                        bool needCreate = false;
                        struct stat filestat;
                        if (::stat(path, &filestat) == ERROR_RESULT_VALUE){
                            needCreate = true;
                        }
                        else{
                            if(!S_ISDIR(filestat.st_mode)){
                                needCreate = true;
                            }
                        }

                        if(needCreate){
                            create_directory_impl(path);
                        }

                        *p = get_separator<char>();
                    }
                }                
            }
            return create_directory_impl(prPathPtr);
        }

        bool open_descriptor(const std::string& path, int &fd_desc, int oflags, mode_t mode)
        {
            if(path.empty()){
                return false;
            }

            bool result = (fd_desc = open(path.c_str(), oflags, mode)) != ERROR_RESULT_VALUE;
            if(!result){
               DEBUG_MSG_PERROR("open_descriptor");
            }

            return result;
        }

        bool open_descriptor(const std::string& path, int &fd_desc, int oflags)
        {
            if(path.empty()){
                return false;
            }

            bool result = (fd_desc = open(path.c_str(), oflags)) != ERROR_RESULT_VALUE;
            if(!result){
               DEBUG_MSG_PERROR("open_descriptor");
            }

            return result;
        }

        bool close_descriptor(int fd_desc)
        {
            if(fd_desc == INVALID_DESCRIPTOR){
                return false;
            }

            bool result = close(fd_desc) != ERROR_RESULT_VALUE;
            if(!result){
               DEBUG_MSG_PERROR("close");
            }
            return result;
        }

        bool write_to_descriptor(int fd_desc, const void *buf, unsigned int len)
        {
            if(fd_desc == INVALID_DESCRIPTOR){
                return false;
            }

            bool result = write(fd_desc, buf, len) != ERROR_RESULT_VALUE;
            if(!result){
                DEBUG_MSG_PERROR("write");
            }
            return result;
        }

        bool read_from_descriptor(int fd_desc, void *buf, unsigned int len, int &readlen)
        {
            if(fd_desc == INVALID_DESCRIPTOR){
                return false;
            }

            readlen = read(fd_desc, buf, len);
            bool result = readlen != ERROR_RESULT_VALUE;
            if(!result){
                DEBUG_MSG_PERROR("read");
            }
            return result;
        }

        off_t get_file_size_by_descriptor(int fd_desc)
        {
            if(fd_desc == INVALID_DESCRIPTOR){
                return 0;
            }

            struct stat stat_buf;
            fstat(fd_desc, &stat_buf);

            return stat_buf.st_size;
        }
    }
}

namespace common
{
    namespace file_system
    {
        Path make_path(const Path& p,const std::string &file_path)
        {
            Path result(p);
            result.append(file_path);
            return result;
        }
        Path make_path_from_uri(const Path& p, const std::string &uri)
        {
            Path result;
            char *dec = url::detail::url_decode(uri.c_str());
            if(dec){
                result = make_path(p,dec);
                free(dec);
            }

            return result;
        }

        Path::Path()
            : is_dir_(INDETERMINATE)
        {

        }

        Path::Path(const std::string &path)
            : is_dir_(file_system::is_directory(path)), path_(isDirectory() ? stable_dir_path(path) : path)
        {
        }

        Path::Path(const Path &other)
            : is_dir_(other.is_dir_), path_(other.path_)
        {

        }

        std::string Path::extension()const
        {
            std::string ext;
            size_t pos = path_.find_first_of('.');
            if(pos != std::string::npos){
                ext = path_.substr(pos+1);
            }
            return ext;
        }

        bool Path::isValid()const
        {
            return is_dir_ != INDETERMINATE;
        }

        bool Path::isFile()const
        {
            return is_dir_ == FAIL;
        }

        bool Path::isDirectory()const
        {
            return is_dir_ == SUCCESS;
        }

        std::string Path::path() const
        {
            return path_;
        }

        bool Path::append(const std::string &path)
        {
            bool is_change=false;
            if(!path.empty()){
                if(path_.empty()){
                    path_ = path;
                    is_change=true;
                }
                else if(isDirectory()){
                    path_ += get_file_name(path);
                    is_change=true;
                }
            }
            if(is_change){
                is_dir_ = file_system::is_directory(path_);
                if(isDirectory()){
                    path_ = stable_dir_path(path_);
                }
            }
            return is_change;
        }

        std::string Path::directory() const
        {
            return get_dir_path(path_);
        }

        File::File(const Path &filePath)
            : path_(filePath), file_(NULL)
        {

        }

        File::~File()
        {
            close();
        }

        bool File::open(const char* mode)
        {
            if(!file_){
                if(path_.isFile()){
                    const char* path = path_.path().c_str();
                    file_ = fopen(path, mode);
                    return file_;
                }
                else{
                    return false;
                }
            }

            return true;
        }

        bool File::read(buffer_type& outData, uint32_t maxSize)
        {
            if(!file_){
                return false;
            }

            byte_type* data = (byte_type*)calloc(maxSize, sizeof(byte_type));
            if(!data){
                return false;
            }

            size_t res = fread(data, sizeof(byte_type), maxSize, file_);
            if(res > 0){
                outData = buffer_type(data, res);
            }
            else if(feof(file_)){

            }
            free(data);

            return true;
        }

        bool File::read(std::string& outData, uint32_t maxSize)
        {
           buffer_type outB;
           bool res = read(outB, maxSize);
           if(res){
               outData = convertToString(outB);
           }
           return res;
        }

        bool File::readLine(buffer_type& outData)
        {
            if(!file_){
                return false;
            }

            char buff[1024] = {0};

            char* res = fgets(buff, sizeof(buff), file_);
            if(res){
                outData = convertFromString<buffer_type>(buff);
            }

            return true;
        }

        bool File::isEof() const
        {
            if(!file_){
                return true;
            }
            return feof(file_);
        }

        bool File::write(const buffer_type& data)
        {
            if(!file_){
                return false;
            }

            if(!data.length()){
                NOTREACHED();
                return false;
            }

            size_t res = fwrite(data.c_str(), sizeof(byte_type), data.length(), file_);
            if(res != data.length()){
                DEBUG_MSG_PERROR("write");
            }
            return res == data.length();
        }

        bool File::write(const std::string& data)
        {
            return write(convertFromString<buffer_type>(data));
        }

        bool File::write(const string16& data)
        {
            return write(convertFromString16<buffer_type>(data));
        }

        void File::flush()
        {
            if(!file_){
                return;
            }
            fflush(file_);
        }

        bool File::isOpened() const
        {
            return file_;
        }

        void File::close()
        {
            if(file_){
                fclose(file_);
                file_ = NULL;
            }
        }
    }
}
