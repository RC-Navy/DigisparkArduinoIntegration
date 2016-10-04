#ifndef RCUL_H
#define RCUL_H

class Rcul
{
  public:
    virtual uint8_t  RculIsSynchro() = 0;
    virtual uint16_t RculGetWidth_us(uint8_t Ch) = 0;
    virtual void     RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
};

#endif