#include <iostream>
#include <math.h>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
  , past_delay_ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  , pointer_ ( 0 )
  , window_size_ ( 40 )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  unsigned int the_window_size = this->window_size_;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return floor(the_window_size);
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }
  int delay = timestamp_ack_received - send_timestamp_acked;
  this->past_delay_[this->pointer_] = delay;
  int i;
  double avgDelay = 0.0;
  for (i = 0; i < 10; ++i) {
    int curDelay = this->past_delay_[(this->pointer_ + i) %10];
    avgDelay = avgDelay + curDelay;
  }
  avgDelay = avgDelay/10;
  if (delay > 128) {
    if (this->window_size_ > 4) {
        this->window_size_ = this->window_size_ - 1.0 / this->window_size_ - avgDelay/50;
        if (this->window_size_ < 4) {
            this->window_size_ = 4;
        }
    }
  } else if (avgDelay < 70) {
    this->window_size_ = this->window_size_ + 1.0/this->window_size_;
  }
  this->pointer_ = (this->pointer_ + 1) % 10;
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
    return 75;
}
