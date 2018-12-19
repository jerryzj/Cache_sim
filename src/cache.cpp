#include "cache.hpp"

Cache::Cache(){
    // reset cache 
    for(auto i : _cache){
        i.reset();
    }
    _bit_block    = 0;
    _bit_line     = 0;
    _bit_set      = 0;
    _bit_tag      = 0;
    _current_line = 0;
    _current_set  = 0;
}

Cache::~Cache(){}

void Cache::read_config(char* config_file){
    ifstream conf;

    conf.open(config_file, ios::in);
    GET_SIZE:{ // Get cache size
        ulint size = 0;
        conf>>size;
        while(size<1 || size>= 262144 || (size&(~size+1)) != size){
            goto GET_SIZE;
        }
        _cache_setting.cache_size = size;
    }
    GET_LINE_SIZE:{ // Get cache line size
        ulint size = 0;
        conf>>size;
        while(size<1 || size>= 262144 || (size&(~size+1)) != size){
            goto GET_LINE_SIZE;
        }
        _cache_setting.block_size = size;
    }
    GET_MAPPING:{ // Get cache mapping policy
        int mapping = -1;
        conf>>mapping;
        switch(mapping){
            case 1:
                _cache_setting.associativity = direct_mapped;
                break;
            case 2:
                _cache_setting.associativity = set_associative;
                break;
            case 3:
                _cache_setting.associativity = full_associative;
                break;
            default:
                goto GET_MAPPING;
        }
    }
    switch(_cache_setting.associativity){
        // Note: replacement policy is not applicable 
        // on a direct mapped cache 
        case direct_mapped:
            _cache_setting.replacement_policy = NONE;
            return;
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
        ulint size = 0;
        conf>>size;
        while(size<1 || size>= 262144 || (size&(~size+1))!=size){
            goto GET_SET;
        }
        _cache_setting.cache_sets = size;
    }
    GET_REPL:{
        int repl = 0;
        conf>>repl;
        switch(repl){
            case 1: 
                _cache_setting.replacement_policy = RANDOM;
                break;
            case 2: 
                _cache_setting.replacement_policy = LRU;
                break;
            default:
                goto GET_REPL;
        }
    }
    conf.close();
}

void Cache::cache_setup(){
    assert(_cache_setting.block_size != 0);
    _cache_setting.num_block = (_cache_setting.cache_size<<10) / _cache_setting.block_size;
    ulint temp = _cache_setting.block_size;
    while(temp){
        temp >>= 1;
        _bit_block++;
    }
    --_bit_block;
    // Setup bit line and bit set
    switch(_cache_setting.associativity){
        case direct_mapped:
            temp = _cache_setting.num_block;
            while(temp){
                temp >>= 1;
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
    int set_count = 0;
    for(int i = 0; i < _cache_setting.num_block; ++i){
        _cache[i][31] = true;
        ++set_count;
    }
    cout<<"Valid bit set = "<<set_count<<endl;


}

void Cache::run_sim(char* trace_file){
    ifstream in_file;
    char trace_line[13];

    in_file.open(trace_file, ios::in);
    while(in_file.fail()){
        cerr<<"Open trace file error"<<endl;
        exit(-1);
    }

    while(!in_file.eof()){
        in_file.getline(trace_line, 13);
        bool __attribute__((unused)) is_success = _CacheHandler(trace_line);
        assert(is_success);
    }
    in_file.close();
    _CalHitRate();
}

void Cache::dump_result(char* trace_file){

    cout<<"Test file: "<<trace_file<<endl;
    cout<<"Cache size: "<<_cache_setting.cache_size<<"KB"<<endl;
    cout<<"Cache block size: "<<_cache_setting.block_size<<"B"<<endl;
    switch(_cache_setting.associativity){
        case direct_mapped:
            cout<<"Associativity: direct_mapped"<<endl;
        break;
        case set_associative:
            cout<<"Associativity: "<<_cache_setting.num_sets<< "-way set_associative"<<endl;
        break;
        case full_associative:
            cout<<"Associativity: fully_associative"<<endl;
        break;
        default:
            cerr<<"Error associtivity setting"<<endl;
            exit(-1);
    }
    switch(_cache_setting.replacement_policy){
        case RANDOM:
            cout<<"Replacement policy: Random"<<endl;
        break;
        case LRU:
            cout<<"Replacement policy: LRU"<<endl;
        break;
        default:
            cerr<<"Error replacement setting"<<endl;
            exit(-1);
    }
    cout<<"\n";
    cout<<"Number of cache access： "<<_counter.access<<endl;
    cout<<"Number of cache load： "  <<_counter.load<<endl;
    cout<<"Number of cache store： " <<_counter.store<<endl;
    cout<<"Cache hit rate: "         <<_counter.avg_hit_rate<<endl;
}


void Cache::_Read(const bitset<32>& addr){
    bool space = false;
    switch(_cache_setting.associativity){
        case direct_mapped:
            if(_cache[_current_line][30] == false){
                _WriteToBlock(addr);
            }
            else{
                _Replace(addr);
            }
        break;
        case full_associative:
            space = false;
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
            space = false;
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

void Cache::_Drop(){
    // Set dirty bit and hit bit to flase
    _cache[_current_line][29] = false;
    _cache[_current_line][30] = false;
}

void Cache::_Replace(const bitset<32>& addr){
    // Find victim block
    switch(_cache_setting.associativity){
        case direct_mapped: // nothing to do 
        break;
        case full_associative:
            if(_cache_setting.replacement_policy == RANDOM){
                _current_line = rand() / (RAND_MAX/_cache_setting.num_block+1);
            }
            else if(_cache_setting.replacement_policy == LRU){
                // Do sth.
            }
            else{
                // _current_line remain the same
            }
        break;
        case set_associative:
            if(_cache_setting.replacement_policy == RANDOM){
                _current_line = rand() / (RAND_MAX/_cache_setting.num_block+1);
            }
            else if(_cache_setting.replacement_policy == LRU){
                // Do sth.
            }
            else{
                // _current_line remain the same
            }
        break;
    }
    // If the victim block is dirty, write back to RAM
    if(_cache[_current_line][29] == true){
        _Drop();
    }
    // Write new data from RAM to Cache 
    _WriteToBlock(addr);
}

bool Cache::_CacheHandler(char* trace_line){
    bool is_load  = false;
    bool is_store = false;
    bool is_space = false;
    bool hit      = false;
    ulint temp = 0;

    switch(trace_line[0]){
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
            cerr<<"Error line: "<<trace_line<<endl;
            return false;
    }
    temp = strtoul(trace_line+2, NULL, 16);
    bitset<32> addr(temp);
    hit = _IsHit(addr);

    if(hit && is_load){
        ++_counter.access;
        ++_counter.load;
        ++_counter.load_hit;
        ++_counter.hit;
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
    }
    else if((!hit) && is_load){
        ++_counter.access;
        ++_counter.load;
        _Read(addr);
    }
    else if((!hit) && is_store){
        ++_counter.access;
        ++_counter.store;
        _Read(addr);
        if(_cache_setting.write_policy == write_back){
            _cache[_current_line][29] = true; // set dirty bit
        }
    }
    else if(is_space){
        ++_counter.space;
    }
    else{
        cerr<<"Unexpected error in _CacheHandler()"<<endl;
        cerr<<"ERROR line: "<<trace_line<<endl;
        return false;
    }
    return true;
}

bool Cache::_IsHit(bitset<32> addr){
    bool ret = false;
    /* switch(_cache_setting.associativity){
        case direct_mapped:
            _current_line = _GetCacheIndex(addr);
            assert(_cache[_current_line][31] == true);
            if(_cache[_current_line][30] == true){
                ret = _CheckIdent(_cache[_current_line],addr);
            }
        break;
        case full_associative:
            for(ulint i = 0; i < _cache_setting.num_block; ++i){
                if(_cache[i][30] == true){
                    ret = _CheckIdent(_cache[i],addr);
                }
                if(ret == true){
                    _current_line = i;
                    break;
                }
            }
        break;
        case set_associative:
            _current_set = _GetCacheIndex(addr);
            ulint i = _cache_setting.num_sets;
            for(int j = i * _current_set; j < (i+1) * _current_set; ++j){
                if(_cache[j][30] == true){
                    ret = _CheckIdent(_cache[j], addr);
                }
                if(ret == true){
                    _current_line = j;
                    break;
                }
            }
        break;
    } */

    if(_cache_setting.associativity == direct_mapped){
        _current_line = _GetCacheIndex(addr);
        cout<<"Current line = "<<_current_line<<endl;
        //assert(_cache[_current_line][31] == true);
        if(_cache[_current_line][30] == true){
            ret = _CheckIdent(_cache[_current_line], addr);
        }
    }
    else if(_cache_setting.associativity == full_associative){
        for(ulint i = 0; i < _cache_setting.num_block; ++i){
            if(_cache[i][30] == true){
                ret = _CheckIdent(_cache[i], addr);
            }
            if(ret == true){
                _current_line = i;
                break;
            }
        }
    }
    else{ // Set associative
        _current_set = _GetCacheIndex(addr);
        ulint i = _cache_setting.cache_sets;
        for(int j = i * _current_set; j < i * (_current_set + 1); ++j){
            if(_cache[j][30] == true){
                ret = _CheckIdent(_cache[j], addr);
            }
            if(ret == true){
                _current_line = j;
                break;
            }
        }
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
    if(_cache_setting.associativity == set_associative){
        for(uint i = _bit_block, j = 0; i < (_bit_block+_bit_set); ++i, ++j){
            temp_cache_line[i] = addr[j];
        }
    }
    else{
        for(uint i = _bit_block, j = 0; i < (_bit_block+_bit_line); ++i, ++j){
            temp_cache_line[i] = addr[j];
        }
    }
    return temp_cache_line.to_ulong();
}

bool Cache::_CheckIdent(const bitset<32>& cache, const bitset<32>& addr){
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
    cout<<"Hit"<<_counter.hit<<endl;
    _counter.avg_hit_rate   = static_cast<double>(_counter.hit) / _counter.access;
    _counter.load_hit_rate  = static_cast<double>(_counter.load_hit) / _counter.load;
    _counter.store_hit_rate = static_cast<double>(_counter.store_hit) / _counter.store; 
}