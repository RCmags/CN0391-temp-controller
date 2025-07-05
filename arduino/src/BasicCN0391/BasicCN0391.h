#ifndef _BasicCN0391_H_
#define _BasicCN0391_H_

constexpr uint8_t NUM_PORT = 4; // Number of ports available in CN0391

// setters
uint8_t setType( char );
char setTypeInv( uint8_t );

// getters
void CN391_getPortType( char [] );
bool CN391_checkPortType( char );

// initialize module
void CN391_setup( char[] );
void CN391_setup( char );

// get temps
void CN391_getTemps( float [], float [], float [], int32_t [] );
void CN391_getThermocoupleTemps(float*);

#endif
