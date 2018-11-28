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
}

void Cache::read_config(){
    GET_SIZE:{ // Get cache size
        #ifdef PROMPT
        cout<<"Please input the number of the cache size(Unit:KB)"<<endl;
        cout<<"\t(for example:1,2,4,8,16,32,64...2^18)"<<endl;
        #endif
        ulint size = 0;
        cin>>size;
        while(size<1 || size>= 262144 || (size&(~size+1)) != size){
            goto GET_SIZE;
        }
        _cache_setting.cache_size = size;
    }
    GET_LINE_SIZE:{ // Get cache line size
        #ifdef PROMPT
        cout<<"Please input the number of the cacheline size(Unit:Byte)"<<endl;
        cout<<"\t(for example:1,2,4,8,16,32,64...2^18)"<<endl;
        #endif
        ulint size = 0;
        cin>>size;
        while(size<1 || size>= 262144 || (size&(~size+1)) != size){
            goto GET_LINE_SIZE;
        }
        _cache_setting.line_size = size;
    }
    GET_MAPPING:{ // Get cache mapping policy
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
                break;
            case 3:
                _cache_setting.mapping_policy = full_associative;
                break;
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
            break;
        default:
            cerr<<"Wrong Mapping Policy Config"<<endl;
            exit(-1);
    }
    GET_SET:{
        #ifdef PROMPT
        cout<<"Input the how many lines in each set:"<<endl;
        cout<<"\t(for example:1,2,4,8,16,32,64...2^18)"<<endl;
        #endif
        ulint size = 0;
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

void Cache::cache_setup(){
    assert(_cache_setting.line_size != 0);
    _cache_setting.num_line = (_cache_setting.line_size<<10) / _cache_setting.line_size;
    ulint temp = _cache_setting.line_size;
    while(temp){
        temp>>=1;
        _bit_block++;
    }
    --_bit_block;
    // Setup  bit line and bit set
    switch(_cache_setting.mapping_policy){
        case direct_mapped:
            temp = _cache_setting.num_line;
            while(temp){
                temp>>=1;
                _bit_line++;
            }
            --_bit_line;
            _bit_set = 0;
            break;
        case full_associative:
            _bit_line = 0;
            _bit_set  = 0;
            break;
        case set_associative:
            _bit_line = 0;
            assert(_cache_setting.cache_sets != 0);
            assert(_cache_setting.num_line > _cache_setting.cache_sets);
            _cache_setting.num_sets = _cache_setting.num_line / _cache_setting.cache_sets;
            temp = _cache_setting.num_sets;
            while(temp){
                temp >>=1;
                _bit_set++;
            }
            --_bit_set;
            break;
        default:
            cerr<<"Invlid mapping policy"<<endl;
            exit(-1);
    }
    _bit_tag = 32ul - _bit_block - _bit_line - _bit_set;
    assert(_bit_tag <= 29);
    // Dump Cache information
    cout<<"Cache line size: "<<_cache_setting.line_size<<"B"<<endl;
    cout<<"Cache size:      "<<_cache_setting.cache_size<<"KB"<<endl;
    switch(_cache_setting.mapping_policy){
        case set_associative:
            cout<<"Cache_set: "<<_cache_setting.cache_sets<<" lines in each set"<<endl;
            cout<<"# of sets: "<<_cache_setting.num_sets<<endl;
        case direct_mapped:
            cout<<"# of bits/line: "<<_bit_line<<endl;
        default:
            cout<<"# of lines: "<<_cache_setting.num_line<<endl;
            cout<<"# of bits/block"<<_bit_block<<endl;
            cout<<"# of bits/tag"<<_bit_tag<<endl;
    }
    for(int i = 0; i < _cache_setting.num_line; ++i){
        _cache[i][31] = true;
    }
}

void Cache::run_test(char* file_path){
    ifstream in_file;
    char address[13];

    in_file.open(file_path, ios::in);
    while(in_file.fail()){
        cerr<<"Open trace file error"<<endl;
        exit(-1);
    }
    while(!in_file.eof()){
        in_file.getline(address, 13);
        bool __attribute__((unused)) is_success = _cache_handler(address);
        assert(is_success);
    }
    _cal_hit_rate();
    in_file.close();
}

bool Cache::_cache_handler(char* address){
    bool is_load  = false;
    bool is_store = false;
    bool is_space = false;
    bool hit      = false;
    ulint temp = 0;

    switch(address[0]){
        case 'l':
            is_load = true;
            break;
        case 's':
            is_store = true;
            break;
        case '\0':
            is_space = true;
            break;
        default:
            cerr<<"Undefined instruction type."<<endl;
            cerr<<"Error in address: "<<address<<endl;
            return false;
    }
    temp = strtoul(address+2,NULL,16);
    
}

void Cache::_cal_hit_rate(){
    assert(_counter.access != 0);
    assert(_counter.load   != 0);
    assert(_counter.store  != 0);
    _counter.avg_hit_rate   = static_cast<double>(_counter.hit) / _counter.access;
    _counter.load_hit_rate  = static_cast<double>(_counter.load_hit) / _counter.load;
    _counter.store_hit_rate = static_cast<double>(_counter.store_hit) / _counter.store; 
}