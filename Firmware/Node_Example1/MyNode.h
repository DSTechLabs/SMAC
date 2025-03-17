//=========================================================
//
//     FILE : MyNode.h
//
//    PROJECT : SMAC Framework - Example 1
//
//    NOTES : This Node has one Devices. It periodically sends
//            a reading from a photoresistor (light sensor)
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef MYNODE_H
#define MYNODE_H

//--- Includes --------------------------------------------

#include "Node.h"


//=========================================================
//  class MyNode
//=========================================================

class MyNode : public Node
{
  public:
    MyNode (const char *name, unsigned char nodeID);
};

#endif
