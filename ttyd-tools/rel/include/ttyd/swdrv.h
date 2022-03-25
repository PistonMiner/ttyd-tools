#pragma once

namespace ttyd::swdrv
{

extern "C" {

void swInit();
void swReInit();

void swSet(int gswf);
bool swGet(int gswf);
void swClear(int gswf);

void swByteSet(int gsw, int value);
int swByteGet(int gsw);

void _swSet(int lswf);
bool _swGet(int lswf);
void _swClear(int lswf);

void _swByteSet(int lsw, int value);
int _swByteGet(int lsw);

}

}