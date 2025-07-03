#ifndef _BasicCN0391_H_
#define _BasicCN0391_H_

constexpr uint8_t NUM_PORT = 4; // Global constant | Number of ports available in CN0391

// Parser
int setType( char );
void CN391_setPortType( int, int );
void CN391_getPortType( int [] );

// initialize module
void CN391_setup( char[] );
void CN391_setup( char );

// get temps
void CN391_getTemps( float [], float [], float [], int32_t [] );
void CN391_getThermocoupleTemps(float*);

#endif
