#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity),
                                                             _capacity(capacity),
                                                             window(capacity),
                                                             recieved(capacity),
                                                             //recieve starting index
                                                             rcv_pos(0),
                                                             _unassembled_bytes(0),
                                                             eof_index(0) {}
                                                              // the index of EOF byte, defined as the next byte of last stream.
                                                            // if eof_index == 0 means no EOF is received.

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // DUMMY_CODE(data, index, eof);
    //start index to push,min(recieved last position , the index)
    size_t start_index = max(rcv_pos,index);
    //end index to push
    size_t end_index = min(rcv_pos+_output.remaining_capacity(),index+data.size());

    //get the total number of the unassembled bytes
    for(size_t i = start_index;i < end_index;i++){
        size_t b = i%_capacity;
        window[b] = data[i-index];
        if(!recieved[b]){
            recieved[b]=1;
            _unassembled_bytes++;
        }
    }
    
    //submit if possible
    size_t pre_end = rcv_pos;
    //统计可push的字节个数 需要保证在容器内
    while(recieved[pre_end % _capacity]&&pre_end < rcv_pos + _capacity)
        pre_end++;

    //待push的结束位置 大于接收开始位置 可以push
    if(pre_end > rcv_pos){
        //待提交长度
        string to_submit(pre_end - rcv_pos,0);

        //将数据从缓冲区 读入 待提交区
        for(size_t i = rcv_pos;i<pre_end;i++){
            to_submit[i-rcv_pos]=window[i % _capacity];
        }
        //write the byte to the bytestream
        size_t written_bytes = _output.write(to_submit);
        //清空 receive 记录
        for(size_t i = rcv_pos;i<rcv_pos+written_bytes;i++){
            recieved[i % _capacity] = 0;
        }
        //更新rcv_pos 
        rcv_pos += written_bytes;
        _unassembled_bytes -= written_bytes;
    }

    //handle eof 
    if(eof){
        eof_index = index + data.size()+1;
    }
    if(_output.bytes_written()+1==eof_index){
        _output.end_input();
    }

}

size_t StreamReassembler::unassembled_bytes() const { return {_unassembled_bytes}; }

bool StreamReassembler::empty() const { return {unassembled_bytes()==0}; }
