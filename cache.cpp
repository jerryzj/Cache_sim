#include "cache.hpp"

Cache::Cache(){
    // reset cache 
    for(auto i : _cache){
        i.reset();
    }
    // reset LRU priority queue
    for(auto i : _LRU_priority){
        i = 0;
    }
    _bit_block    = 0;
    _bit_line     = 0;
    _bit_set      = 0;
    _bit_tag      = 0;
    _current_line = 0;
    _current_set  = 0;
};

void Cache::read_config(){
    ulint size = 0;
GET_SIZE:{
    #ifdef PROMPT
    cout<<"Please input the number of the cache size(Unit:KB)"<<endl;
    cout<<"\t(for example:1,2,4,8,16,32,64...2^18)"<<endl;
    #endif
    cin>>size;
    while(size<1 || size>= 262144 || (size&(~size+1)) != size){
        goto GET_SIZE;
    }
    _cache_setting.cache_size = size;
}
GET_LINE_SIZE:{
    #ifdef PROMPT
    cout<<"Please input the number of the cacheline size(Unit:Byte)"<<endl;
    cout<<"\t(for example:1,2,4,8,16,32,64...2^18)"<<endl;
    #endif
    cin>>size;
    while(size<1 || size>= 262144 || (size&(~size+1)) != size){
        goto GET_LINE_SIZE;
    }
    _cache_setting.line_size = size;
}
GET_MAPPING:{
    #ifdef PROMPT
    cout<<"Please input the method of assoiativity between main memory and cache:"<<endl;
    cout<<"\t directive_mapped: 1"<<endl;
    cout<<"\t set_associative : 2"<<endl;
    cout<<"\t full_associative: 3"<<endl;
    #endif
    int mapping = -1;
    cin>>mapping;
    switch(mapping){
        case 1:
            _cache_setting.mapping_policy = direct_mapped;
            break;
        case 2:
            _cache_setting.mapping_policy = set_associative;
        case 3:
            _cache_setting.mapping_policy = full_associative;
        default:
            goto GET_MAPPING;
    }
}
    switch(_cache_setting.mapping_policy){
        case direct_mapped:
            _cache_setting.replacement_policy = NONE;
            goto GET_WRITE;
            break;
        case full_associative:
            goto GET_REPL;
            break;
        case set_associative:
            goto GET_SET;
        default:
            cerr<<"Wrong Mapping Policy Config"<<endl;
            exit(-1);
    }
GET_SET:{
    #ifdef PROMPT
    cout<<"Input the how many lines in each set:"<<endl;
    cout<<"\t(for example:1,2,4,8,16,32,64...2^18)"<<endl;
    #endif
    cin>>size;

    while(size<1 || size>= 262144 || (size&(~size+1))!=size){
        goto GET_SET;
    }
    _cache_setting.cache_sets = size;
}
GET_REPL:{
    #ifdef PROMPT
    cout<<"Please input the replacement policy:"<<endl;
    cout<<"\t FIFO(First In First Out):input 1"<<endl;
    cout<<"\t LRU(Least Recently Used):input 2"<<endl;
    cout<<"\t LFU(Least Frequently Used):input 3"<<endl;
    cout<<"\t Random:input 4"<<endl;
    #endif
    int repl = 0;
    cin>>repl;
    switch(repl){
        case 1: 
            _cache_setting.replacement_policy = FIFO;
            break;
        case 2: 
            _cache_setting.replacement_policy = LRU;
            break;
        case 3:
            _cache_setting.replacement_policy = LFU;
            break;
        case 4:
            _cache_setting.replacement_policy = RANDOM;
            break;
        default:
            goto GET_REPL;
    }
}
GET_WRITE:{
    #ifdef PROMPT
    cout<<"Please input write policy:"<<endl;
    cout<<"\t Write through:input 1"<<endl;
    cout<<"\t Write back:input 2"<<endl;
    #endif
    int write = -1;
    cin>>write;
    switch(write){
        case 1: 
            _cache_setting.write_policy = write_through;
            break;
        case 2:
            _cache_setting.write_policy = write_back;
            break;
        default:
            goto GET_WRITE;
    }
}
}