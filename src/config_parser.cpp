#include "config_parser.hpp"

CACHE_SET readConfig(char* config_file){
    CACHE_SET _cache_conf;
    std::list<string> config_raw, config_ready;

    // Preprocessing config file
    config_raw = readFile(config_file);
    config_ready = removeComments(config_raw);

    // Assert config lines is in the right range
    assert (("Config size should be 3~5 lines", config_ready.size() >= 3 && config_ready.size <=5));

    // Read cache size
    // WIP: use new API, add assert message
    assert(readParameter(config_ready.front(), _cache_conf.cache_size));
    config_ready.pop_front();
    
    // Read cache line size
    // WIP: use new API, add assert message
    assert(readParameter(config_ready.front(), _cache_conf.block_size));
    config_ready.pop_front();


    // Read cache associativity
    switch(stoi(config_ready.front())){
        case 1:
            _cache_conf.associativity = direct_mapped;
            break;
        case 2:
            _cache_conf.associativity = set_associative;
            break;
        case 3:
            _cache_conf.associativity = full_associative;
            break;
        default:
            cerr<<"Invalid Associativity"<<endl;
            std::exit(-1);
    }
    config_ready.pop_front();

    if(_cache_conf.associativity == direct_mapped) {
        // Note that replacement policy is invalid for direct-mapped cache
        _cache_conf.replacement_policy = NONE;
        dumpCacheConf(_cache_conf);
        return _cache_conf;
    }
    else if (_cache_conf.associativity == set_associative) {
        // Read set size
        // WIP: use new API, add assert message
        assert(readParameter(config_ready.front(), _cache_conf.cache_sets));
        config_ready.pop_front();

    }
    else{
        // TODO: For fully-associative cache
    }
    dumpCacheConf(_cache_conf);
    return _cache_conf;
}


void dumpCacheConf(const CACHE_SET& cache_config){

}

bool sizeCheck(ulint size) {
    return (size<1) || (size>=262144) || (size&(~size+1)) != size;
}

bool readParameter(const std::string& conf, ulint& para) {
    para = std::stoull(conf.c_str());
    if(sizeCheck(para)){
        return true;
    }
    return false;
}

std::list<std::string> readFile(char* config_file){
    ifstream file;
    string temp;
    list<string> ans;

    file.open(config_file, ios::in);
    while(getline(file, temp)){
        temp.erase(remove(temp.begin(), temp.end(), '\r'), temp.end());
        ans.push_back(temp);
    }
    file.close();
    return ans;
}

std::list<std::string> removeComments(const std::list<std::string>& source) {
    std::list<std::string> ans;
    bool status = 0; // false:nothing, true:under block comment
    for(std::string line : source){
        if(status == 0 && (ans.empty() || ans.back()!="")) {
            ans.push_back("");
        }
        int i=0;
        while(i < (int) line.size()) {
            if(line.substr(i,2) == "//") {
                if(status == 0)
                    break; //break  while(i<line.size()){...}   loop
            }
            else if(line.substr(i,2) == "/*") {
                if(status == 0){
                    status = 1;
                    i+=2;
                    continue;
                }
            }
            else if(line.substr(i,2) == "*/") {
                if(status == 1){
                    status = 0;
                    i+=2;
                    continue;
                }
            }
            if(status == 0) {
                ans.back().push_back(line[i]);
            }
            i++;
        }
    }
    return ans;
}