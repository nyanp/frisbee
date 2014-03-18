#include <iostream>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/predef.h>
#include <boost/utility.hpp>
#include "picojson.h"

namespace frisbee
{
    inline std::string lang2name(const std::string& lang) {
        if (lang == "C++") return "gcc-head";
        if (lang == "C") return "gcc-4.8.2-c";
        if (lang == "D") return "gdc-head";
        if (lang == "haskell") return "ghc-7.6.3";
        if (lang == "C#") return "mcs-head";
        if (lang == "perl") return "perl-head";
        if (lang == "python") return "python-head";
        if (lang == "ruby") return "ruby-head";
        if (lang == "mruby") return "mruby-head";
        if (lang == "php") return "php-head";
        if (lang == "erlang") return "erlang-head";
        if (lang == "javascript") return "node-head";
        if (lang == "coffeescript") return "coffee-script-head";
        if (lang == "sql") return "sqlite-head";
        if (lang == "rust") return "rust-head";
        if (lang == "sql") return "sqlite-head";
        if (lang == "sql") return "sqlite-head";
        if (lang == "bash") return "bash";
        if (lang == "lua") return "lua-5.2.2";
        if (lang == "lazyk") return "lazyk";
        if (lang == "lisp") return "clisp-2.49.0";
        if (lang == "pascal") return "fpc-2.6.2";
        if (lang == "java") return "java7-openjdk";
        if (lang == "groovy") return "groovy-2.2.1";
        throw std::runtime_error("language not found");
    }

    inline std::string json_escape(const std::string& src) {
        std::string targ(src);
#if BOOST_OS_WINDOWS
        boost::replace_all(targ, "<", "^<"); // for windows
        boost::replace_all(targ, ">", "^>"); // for windows
#endif
        boost::replace_all(targ, "\"", "\\\"");
        boost::replace_all(targ, "\\\\\"", "\\\\\\\"");

        return targ;
    }

    class compile_result {
    public:
        explicit compile_result(picojson::value wandbox_response) {
            picojson::object& o = wandbox_response.get<picojson::object>();

            if (o.find("error") != o.end()) throw std::runtime_error("error-response:" + o["error"].serialize());

#define GET(name ,type) if (o.find(#name) != o.end()) name##_ = boost::lexical_cast<type>(o[#name].get<std::string>());
            GET(status, int);
            GET(signal, std::string);
            GET(compiler_output, std::string);
            GET(compiler_error, std::string);
            GET(program_output, std::string);
            GET(program_error, std::string);
#undef GET
        }

        bool is_error() const { return !compiler_error_.empty(); }
        std::string compiler_error() const { return compiler_error_; }

        template<typename T>
        T get_result() const { return boost::lexical_cast<T>(program_output_); }

    private:
        boost::optional<int> status_;
        std::string signal_;
        std::string compiler_output_;
        std::string compiler_error_;
        std::string program_output_;
        std::string program_error_;
    };

#if BOOST_OS_WINDOWS
#define popen _popen
#define pclose _pclose
#define fileno _fileno
#endif

    // RAII wrapper for popen/pclose
    class pfile : boost::noncopyable {
    public:
        explicit pfile(const std::string& filename) : fp_(popen(filename.c_str(), "r")) {
            if (!fp_) throw std::runtime_error("popen failed");
        }
        int file_descriptor() const { return fileno(fp_); }
        ~pfile() { pclose(fp_); }
    private:
        FILE *fp_;
    };

    class curl {
    public:
        explicit curl(const std::string& url) : url_(url) {}
        void add_data(const std::string& data) { data_.push_back(data); }
        void add_header(const std::string& header) { header_.push_back(header); }

        picojson::value get_response() const {
            pfile file(make_curl_cmd());

            descriptor_source pipe_device(file.file_descriptor(), boost::iostreams::never_close_handle);
            boost::iostreams::stream<descriptor_source> pipe_stream(pipe_device, 0x1000, 0x1000);

            picojson::value result;
            std::string error;

            picojson::parse(result,
                std::istreambuf_iterator<char>(pipe_stream),
                std::istreambuf_iterator<char>(),
                &error);

            if (!error.empty())
                throw std::runtime_error("failed to parse json:" + error);

            return result;
        }
    private:
        typedef boost::iostreams::file_descriptor_source descriptor_source;

        std::string make_curl_cmd() const {
            std::ostringstream os;
            os << "curl -s -S "; // without progress
            for (auto h : header_) os << "-H \"" << h << "\" ";
            for (auto d : data_)   os << "-d \"" << d << "\" ";
            os << url_;
            return os.str();
        }
        std::string url_;
        std::vector<std::string> data_;
        std::vector<std::string> header_;
    };

    inline compile_result compile(const std::string& lang, const std::string& source_code) {
        curl curl("http://melpon.org/wandbox/api/compile.json");

        picojson::object o;
        o["code"] = (picojson::value)source_code;
        o["compiler"] = (picojson::value)lang2name(lang);

        curl.add_data(json_escape(picojson::value(o).serialize()));
        curl.add_header("Content-type: application/json");
        return compile_result(curl.get_response());
    }

    inline compile_result compile_fromfile(const std::string& lang, const std::string& filename) {
        std::ifstream ifs(filename);
        if (ifs.fail() || ifs.bad())
            throw std::runtime_error("failed to open:" + filename);

        std::string code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        return compile(code, lang);
    }

    template<typename T>
    T eval(const std::string& lang, const std::string& source_code) {
        compile_result r = compile(lang, source_code);
        if (r.is_error())
            throw std::runtime_error("failed to compile:\n" + r.compiler_error());
        return r.get_result<T>();
    }
} // namespace frisbee
