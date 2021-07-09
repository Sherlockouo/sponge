#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

// ByteStream::ByteStream(const size_t capacity):capacity(capacity),buffer(capacity,0),readed_bytes(0),writed_bytes(0),is_end(0) { DUMMY_CODE(capacity); }
ByteStream::ByteStream(const size_t capacity):_capacity(capacity),readed_bytes(0),writed_bytes(0),buf(0),is_end(false) {
    //buf should be empty at first
    //  DUMMY_CODE(capacity); 
    }

size_t ByteStream::write(const string &data) {
    
    /*
    To see if this is buffer is full or the data is end
    if true it cant write any more data to the buffer
    */
    // if(readed_bytes==_capacity||is_end){
    //     return 0;
    // }
    //the size is the buffer has used
    size_t used = buf.size();
    //to ensure the buffer still have the space to store the data
    size_t size = min(data.size(),_capacity-used);
    //to ensure the size not negative
    size = max(size,size_t(0));

    //write the data to the buffer
    for(size_t i=0;i<size;i++){
        buf.push_back(data[i]);
    }
    writed_bytes+=size;

    return {size};
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    //take out len size of buffer
    size_t size = min(len,buf.size());
    string res;

    for(size_t i=0;i<size;i++){
        res+=buf[i];
    }

    return {res};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {

    size_t size = min(len,buf.size());

    for(size_t i=0;i<size;i++){
        buf.erase(buf.begin());
    }

    //readed bytes ++
    readed_bytes += size;

}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string res;

    // below two operations doesn't change the size of the buffer
    // and this is a sigle thread version of TCP so its ok two do this
    res = peek_output(len);
    pop_output(len);

    return {res};
}

//上级调用 结束输入
void ByteStream::end_input() {
    is_end = 1;
}

//输入是否结束
bool ByteStream::input_ended() const {
    return {is_end}; 
}

//缓冲区大小
size_t ByteStream::buffer_size() const { return {buf.size()}; }

//缓冲区是否为空
bool ByteStream::buffer_empty() const { return {buf.empty()}; }

//是否到文件尾 需要结束输入 且 缓冲区为空
bool ByteStream::eof() const {
    // sll's idea
    return is_end&&buf.empty(); 
    // buffer is empty and this should be eof.
    // return buf.empty(); 
}

//一共写了多少到缓冲区
size_t ByteStream::bytes_written() const { return {writed_bytes}; }

//一共读出多少字节
size_t ByteStream::bytes_read() const { return {readed_bytes}; }

//
size_t ByteStream::remaining_capacity() const { return {_capacity-buf.size()}; }
