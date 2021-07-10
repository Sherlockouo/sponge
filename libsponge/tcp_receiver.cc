#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//receive segments 
void TCPReceiver::segment_received(const TCPSegment &seg) {
    uint64_t prev_abs_ackno = 0;
    //处理第一个 syn
    if(seg.header().syn && !isn.has_value()){
        //如果header 中有 syn 且 当前 isn 为空 就可以设置 isn 为
        isn = make_optional<WrappingInt32>(seg.header().seqno);
    }
    //如果isn 有值 那么 就有
    if(isn.has_value()){
        prev_abs_ackno = abs_ackno();
    }else{
        return ;
    }

    uint64_t checkpoint = _reassembler.stream_out().bytes_written();

    //将seqno 包装为 abs_seq
    uint64_t abs_seq = unwrap(seg.header().seqno,isn.value(),checkpoint);

    uint64_t prev_window_size = window_size();

    //ACK after FIN received
    if(abs_fin_seq && abs_seq >= abs_fin_seq && seg.length_in_sequence_space()==0){
        
        return ;
    }

    if(!(abs_seq < prev_abs_ackno + prev_window_size && abs_seq + seg.length_in_sequence_space() > prev_abs_ackno)){
        //dont know how
        return ;
    }

    bool all_filled = abs_seq + seg.length_in_sequence_space() <= prev_abs_ackno + prev_window_size;

    if(all_filled && seg.header().fin){
        abs_fin_seq = abs_seq + seg.length_in_sequence_space();
    }


    uint64_t stream_indices = abs_seq > 0?abs_seq - 1: 0;
    string payload (seg.payload().copy());
    _reassembler.push_substring(payload,stream_indices,stream_indices+seg.payload().size() + 2==abs_fin_seq);

    return ;


}

uint64_t TCPReceiver::abs_ackno() const{
    uint64_t abs_ackno_without_fin = 1 + _reassembler.stream_out().bytes_written();
    //如果获取到fin了 那么就结束了 返回ackno的绝对位置
    if(abs_ackno_without_fin + 1 == abs_fin_seq){
        return abs_ackno_without_fin+1;
    }
    return abs_ackno_without_fin;
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(!isn.has_value()){
        return nullopt;
    }
    return make_optional<WrappingInt32>(wrap(abs_ackno(),isn.value())); 
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
