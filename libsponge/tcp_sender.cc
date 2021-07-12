#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _total_flying_size(0)
    , _receive_window_size(1)
    , _receive_window_edge(1)
    , _timer_countdown(0)
    , _timer_started(false)
    , _RTO(_initial_retransmission_timeout)
    , _consecutive_retransmission(0)
    , _latest_abs_ackno(0)
    , _abs_fin_seq(0)
    {}


uint64_t 
TCPSender::bytes_in_flight() const { return _total_flying_size; }

//! 尽可能发送更多数据 fill in the window
void TCPSender::fill_window() {
    //config send_size with the max payload size
    size_t send_size = TCPConfig::MAX_PAYLOAD_SIZE;

    //define a segment
    TCPSegment segment;

    //! set segment seqno before modify internal data structure
    //! initially the next_seqno() is zero
    segment.header().seqno=next_seqno();

    //todo why the _next_seqno==0 means that SYN is not been sent?
    if(_next_seqno==0){ // SYN not sent yet
        send_size=0;
        segment.header().syn = true;
        _next_seqno++;
    }

    //! when window == 0, assume the remote receive window size is 1 
    //! so that receiver may openup its window if he has more space
    size_t remote_window_edge = _receive_window_edge+
    (_receive_window_size==0 && _receive_window_edge == _next_seqno);
    //! adjust remote receive window
    send_size = min(send_size,remote_window_edge-_next_seqno);

    //read the bytes to the segment's payload
    segment.payload() = Buffer(_stream.read(send_size));
    //send_size may be bigger than the capacity
    send_size = min(send_size,segment.payload().size());

    _next_seqno += send_size;

    //! no data needs to be sent and receiver have window to receive FIN
    if(_stream.eof() && _abs_fin_seq == 0 && _next_seqno < remote_window_edge){
        //! set fin flag
        segment.header().fin = true;
        //! fin is on the last byte's position
        _abs_fin_seq = _next_seqno;
        _next_seqno++;
    }

    _total_flying_size += segment.length_in_sequence_space();

    if(segment.length_in_sequence_space()==0){
        return ;
    }
    
    //! keep track of the segment
    _flying_segments.push(segment);
    //! send the segment
    _segments_out.push(segment);
    //! if timer is not started starts it
    if(!_timer_started){
        _timer_started = true;
        _timer_countdown = _RTO;
    }

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t abs_ackno = unwrap(ackno,_isn,_next_seqno);

    // received ackno that not sent yet
    // note that abs_ackno is seq_no that receiver want to receive
    // so that mean receiver doesn't received the ack 
    if(abs_ackno > _next_seqno){
        return ;
    }

    //update the window_size
    _receive_window_size = window_size;
    _receive_window_edge = abs_ackno + window_size;
    
    // ack already acked segment
    if(abs_ackno <= _latest_abs_ackno){
        return ;
    }

    // reset RTO
    _RTO = _initial_retransmission_timeout;
    // reset timer
    _timer_countdown = _RTO;
    // reset the count
    _consecutive_retransmission = 0;

    auto new_acked_data_size = abs_ackno - _latest_abs_ackno;

    _total_flying_size -= new_acked_data_size;

    if(_total_flying_size == 0){
        _timer_started = false;
    }

    // clear the retransmission queue
    while(!_flying_segments.empty()){
        TCPSegment seg = _flying_segments.front();
        uint64_t abs_seq = unwrap(seg.header().seqno,_isn,_next_seqno);
        if(abs_seq + seg.length_in_sequence_space() <= abs_ackno){
            _flying_segments.pop();
        }else{
            break;
        }   
    }

    // update _latest_abs_ackno
    _latest_abs_ackno = abs_ackno;
    return ;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    //! if timer is not started,cant tick
    if(!_timer_started){
        return ;
    }

    _timer_countdown -= ms_since_last_tick;

    //! timer not expired ,return;
    if(_timer_countdown>0){
        return ;
    }

    // 1 retransmit the earliest segment that is not fully acked
    _segments_out.push(_flying_segments.front());
    // 2 if window size is nonzero
    if(_receive_window_size > 0 ){
        //
        _consecutive_retransmission++;
        // exponential back off
        _RTO*=2;
    }
    // 3 start the retransmission timer
    _timer_countdown = _RTO;

}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission; }

void TCPSender::send_empty_segment() {
    TCPSegment segment;
    segment.header().seqno = next_seqno();
    _segments_out.push(segment);

    // doesn't need to be kept track track of the number of consecutive retransmissions
}
