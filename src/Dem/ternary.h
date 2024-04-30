#include <cstdlib>
#include <cstdio>
#include <iostream>

class ternary {
public: 
  int operator[] (int i) 
  {
    if (i==special_quat_bit) // Special bit returns: 00=>0, 10=>1, 11=>-1, like normal bit, and 01=>+2 if bit 0==-1 ; 01=>-2 if bit0==+1 
    {
      if ((bit1>>i)&1)
      {
        if ((bit2>>i)&1)
          return -1 ; 
        else
          return 1 ; 
      }
      else
      {
        if ((bit2>>i)&1)
          return 2 ; 
        else
          return 0 ; 
      }
    }
    else
      return ((bit2>>i)&1) ? -1: ((bit1>>i) & 1) ;  // state bit2=1 and bit1=0 is no legal. 00 => 0, 10=>1, 11=> -1, 01=>illegal. 
  }
  
  void operator++ (int a [[maybe_unused]])
  {
  int i=0 ; 
  bool carry=false ; 
  do {
    carry = false ; 
    if ( !((bit1>>i)&1) ) 
    {
      if (i==special_quat_bit && !((bit2>>i)&1))
      {
        bit2 |= 1<<i ;
      }
      else
      {
        bit1 |= (1<<i) ; 
        bit2 &= ~(1<<i) ; 
      }
    }
    else
    {
      if ( !((bit2>>i)&1) ) 
      {
        bit2 |= 1<<i ;
      }
      else
      {
        bit1 &= ~(1<<i) ; 
        bit2 &= ~(1<<i) ; 
        carry = true ; 
      }
    } 
    i++ ; 
  } while (carry && i<32) ; 
  }
  
  bool operator< (unsigned int n) 
  {
    return (to_integer()<n) ; 
  }
    
  void set(int i, int v)
  {
    if (v==0)
    {
      bit1 &= ~(one<<i) ;      
      bit2 &= ~(one<<i) ;
    }
    else if (v<0) 
    {
      bit1 |= (one<<i) ; 
      bit2 |= (one<<i) ; 
    }
    else if (v>0)
    {
      bit1 |=  (one<<i) ; 
      bit2 &= ~(one<<i) ; 
    }  
    else
      printf("ERR a number needs to be positive or negative or zero ... \n") ; 
  }
  
  void set_quat_bit (int n) {special_quat_bit = n ; }
  void reset_quat_bit () {special_quat_bit = 255 ; }
  
  void clear ()
  {
    bit1=bit2=0 ; 
  }
  
  uint64_t to_integer ()
  {
    int base = 1 ; 
    uint64_t sum = 0 ; 
    auto b1 = bit1, b2=bit2 ;
    while (b1)
    {
      if (b1&1)
      {
        if (b2&1)
          sum += base * 2 ; 
        else
          sum += base ; 
      }
      b1>>=1 ; 
      b2>>=1 ;
      base *= 3 ; 
    }
    return sum ; 
  }
  
  friend std::ostream& operator<< (std::ostream& stream, ternary& t) {
    char buf[33] ; buf[32]=0 ;
    for (int i=31 ; i>=0 ; i--)
    {
      if ((t.bit1>>i)&1)
      {
        if ((t.bit2>>i)&1)
          buf[31-i] = '2' ; 
        else
          buf[31-i] = '1' ;
      }
      else
      {
        if ((t.bit2>>i)&1)
        {
          if (i==t.special_quat_bit)
            buf[31-i] = '3' ; 
          else
            buf[31-i] = '?' ; 
        }
        else
          buf[31-i] = '0' ;
      }
    }
    stream << buf ; 
    return stream ; 
  }
  
private:
  uint32_t bit1=0, bit2=0 ; 
  const uint32_t one = 1 ; 
  uint8_t special_quat_bit = 255 ; 
} ;
