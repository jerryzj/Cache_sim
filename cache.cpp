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
        _cache_setting.block_size = size;
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
    assert(_cache_setting.block_size != 0);
    _cache_setting.num_block = (_cache_setting.block_size<<10) / _cache_setting.block_size;
    ulint temp = _cache_setting.block_size;
    while(temp){
        temp>>=1;
        _bit_block++;
    }
    --_bit_block;
    // Setup  bit line and bit set
    switch(_cache_setting.mapping_policy){
        case direct_mapped:
            temp = _cache_setting.num_block;
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
            assert(_cache_setting.num_block > _cache_setting.cache_sets);
            _cache_setting.num_sets = _cache_setting.num_block / _cache_setting.cache_sets;
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
    cout<<"Cache line size: "<<_cache_setting.block_size<<"B"<<endl;
    cout<<"Cache size:      "<<_cache_setting.cache_size<<"KB"<<endl;
    switch(_cache_setting.mapping_policy){
        case set_associative:
            cout<<"Cache_set: "<<_cache_setting.cache_sets<<" lines in each set"<<endl;
            cout<<"# of sets: "<<_cache_setting.num_sets<<endl;
        case direct_mapped:
            cout<<"# of bits/line: "<<_bit_line<<endl;
        default:
            cout<<"# of lines: "<<_cache_setting.num_block<<endl;
            cout<<"# of bits/block"<<_bit_block<<endl;
            cout<<"# of bits/tag"<<_bit_tag<<endl;
    }
    for(int i = 0; i < _cache_setting.num_block; ++i){
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

#ifdef LOG
    int line_processed = 0;
    ofstream log_file;
    log_file.open("cache_sim.log",ios::out);
#endif
    while(!in_file.eof()){
        in_file.getline(address, 13);
        // 1201:Maybe add log_file ref to _CacheHandler
        bool __attribute__((unused)) is_success = _CacheHandler(address);
        assert(is_success);
#ifdef LOG
        ++line_processed;
        log_file<<line_processed;
        log_file<<"ADDR:"<<address<<endl;
#endif
    }
    _CalHitRate();
    in_file.close();
#ifdef LOG
    log_file.close();
#endif
}

void Cache::_Read(const bitset<32>& addr){
    switch(_cache_setting.mapping_policy){
        case direct_mapped:
            if(_cache[_current_line][30] == false){
                _WriteToBlock(addr);
            }
            else{
                _Replace(addr);
            }
        break;
        case full_associative:
            bool space = false;
            // Find available block
            for(uint i = 0; i < _cache_setting.num_block; ++i){
                if(_cache[i][30] == false){
                    space = true;
                    _current_line = i;
                    break;
                }
            }
            if(space == true){
                _WriteToBlock(addr);
                if(_cache_setting.replacement_policy == LRU){
                    // need extra LRU hit handler
                }
            }
            else{
                _Replace(addr);
            }
        break;
        case set_associative:
            bool space = false;
            int i = _cache_setting.cache_sets;
            for(int j = (_current_set * i); i < (_current_set+1) * i;j++){
                if(_cache[i][30] == false){
                    space = true;
                    _current_line = j;
                    break;
                }
            }
            if(space == true){
                _WriteToBlock(addr);
                if(_cache_setting.replacement_policy == LRU){
                    // need extra LRU hit handler
                }
            }
            else{
                _Replace(addr);
            }
        break;
    }
}

void Cache::_Write(){
    // Set dirty bit and hit bit to flase
    _cache[_current_line][29] = false;
    _cache[_current_line][30] = false;
}

bool Cache::_CacheHandler(char* address){
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
    bitset<32> flag(temp);
    hit = _IsHit(flag);

    if(hit && is_load){
        ++_counter.access;
        ++_counter.load;
        ++_counter.load_hit;
        ++_counter.hit;

        if(_cache_setting.replacement_policy == LRU){
            _LRUHitHandler();
        }
    }
    else if(hit && is_store){
        ++_counter.access;
        ++_counter.store;
        ++_counter.store_hit;
        ++_counter.hit;

        // If write back, we need to set dirty bit
        if(_cache_setting.write_policy == write_back){
            _cache[_current_line][29] = true;
        }
        if(_cache_setting.replacement_policy == LRU){
            _LRUHitHandler();
        }
    }
    else if((!hit) && is_load){
        ++_counter.access;
        ++_counter.load;
    }
    else if((!hit) && is_store){
        ++_counter.access;
        ++_counter.store;
        if(_cache_setting.write_policy == write_back){
            _cache[_current_line][29] = true; // set dirty bit
        }
    }
    else if(is_space){
        ++_counter.space;
    }
    else{
        cerr<<"Unexpected error in _CacheHandler"<<endl;
        cerr<<"ADDR: "<<address<<endl;
        return false;
    }
    return true;
}

bool Cache::_IsHit(bitset<32> flag){
    bool ret = false;
    switch(_cache_setting.mapping_policy){
        case direct_mapped:
            _current_line = _GetCacheIndex(flag);
            assert(_cache[_current_line][31] ==  true);
            if(_cache[_current_line][30] == true){
                ret = _CheckAddrIdent(_cache[_current_line],flag);
            }
        break;
        case full_associative:
            for(ulint i = 0; i < _cache_setting.num_block; ++i){
                if(_cache[i][30] == true){
                    ret = _CheckAddrIdent(_cache[i],flag);
                }
                if(ret == true){
                    _current_line = i;
                    break;
                }
            }
        break;
        case set_associative:
            _current_set = _GetCacheIndex(flag);
            ulint i = _cache_setting.num_sets;
            for(int j = i * _current_set; j < (i+1) * _current_set; ++j){
                if(_cache[j][30] == true){
                    ret = _CheckAddrIdent(_cache[j],flag);
                }
                if(ret == true){
                    _current_line = j;
                    break;
                }
            }
        break;
        default: 
            cerr<<"Undefined Cache Mapping"<<endl;
            exit(-1);
    }
    return ret;
}


void  Cache::_WriteToBlock(const bitset<32>& addr){
    for(uint i = 31, j = 28; i > (32ul-_bit_tag); --i, --j){
        _cache[_current_line][j] =  addr[i];
        assert(j > 0);
    }
    _cache[_current_line][30] = true;
}

ulint Cache::_GetCacheIndex(const bitset<32>& addr){
    bitset<32> temp_cache_line;
    for(uint i = _bit_block, j = 0; i < (_bit_block+_bit_line); ++i, ++j){
        temp_cache_line[i] = addr[j];
    }
    return temp_cache_line.to_ulong();
}

bool Cache::_CheckAddrIdent(const bitset<32>& cache, const bitset<32>& addr){
    for(uint i = 31, j = 28; i > (31ul-_bit_tag); --i, --j){
        if(addr[i] != cache[j]){
            return false;
        }
    }
    return true;
}

void Cache::_CalHitRate(){
    assert(_counter.access != 0);
    assert(_counter.load   != 0);
    assert(_counter.store  != 0);
    _counter.avg_hit_rate   = static_cast<double>(_counter.hit) / _counter.access;
    _counter.load_hit_rate  = static_cast<double>(_counter.load_hit) / _counter.load;
    _counter.store_hit_rate = static_cast<double>(_counter.store_hit) / _counter.store; 
}