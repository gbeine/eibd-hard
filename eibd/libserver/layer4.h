/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef LAYER4_H
#define LAYER4_H

#include "layer3.h"

/** information about a broadcast packet */
class BroadcastComm: public LoggableObjectInterface
{
public:
  /** Layer 4 data */
  CArray data;
  /** source address */
  eibaddr_t src;
};

/** informations about a group communication packet */
class GroupComm: public LoggableObjectInterface
{
public:
  /** Layer 4 data */
  CArray data;
  /** source address */
  eibaddr_t src;
};

/** a raw layer 4 connection packet */
class TpduComm:   public LoggableObjectInterface
{
public:
  /** Layer 4 data */
  CArray data;
  /** individual address of the remote device */
  eibaddr_t addr;
} ;

/** informations about a group communication packet */
class GroupAPDU :   public LoggableObjectInterface
{
public:
  /** Layer 4 data */
  CArray data;
  /** source address */
  eibaddr_t src;
  /** destination address */
  eibaddr_t dst;
} ;

/** a class allowing carray to behave like loggable object */
class IndivPDU: public CArray, public LoggableObjectInterface
{
};

/** Broadcast Layer 4 connection */
class T_Broadcast:public L_Data_CallBack, public DroppableQueueInterface,  public LoggableObjectInterface
{
  /** Layer 3 interface */
  Layer3 *layer3;
  /** output queue */
    Queue < BroadcastComm > outqueue;
    /** semaphore for output queue */
  pth_sem_t sem;

  bool init_ok;
  const static char outdropmsg[], indropmsg[];

public:
  T_Broadcast (Layer3 * l3,
	       Logs * t, int write_only,
	       int inquemaxlen, int outquemaxlen, int peerquemaxlen);
    virtual ~ T_Broadcast ();
  bool init ();

  void Get_L_Data (L_Data_PDU * l);

  /** receives APDU of a broadcast; aborts with NULL if stop occurs */
  BroadcastComm *Get (pth_event_t stop);
  /** send APDU c */
  void Send (const CArray & c);
};

/** Group Communication socket */
class GroupSocket:public L_Data_CallBack, public DroppableQueueInterface,  public LoggableObjectInterface
{
  /** Layer 3 interface */
  Layer3 *layer3;
  /** output queue */
    Queue < GroupAPDU > outqueue;
    /** semaphore for output queue */
  pth_sem_t sem;

  bool init_ok;
  const static char outdropmsg[], indropmsg[];

public:
  GroupSocket (Layer3 * l3, Logs * t,   int write_only,
	       int inquemaxlen, int outquemaxlen, int peerquemaxlen);
  virtual ~ GroupSocket ();
  bool init();

  void Get_L_Data (L_Data_PDU * l);

  /** receives APDU of a broadcast; aborts with NULL if stop occurs */
  GroupAPDU *Get (pth_event_t stop);
  /** send APDU c */
  void Send (const GroupAPDU & c);
};

/** Group Layer 4 connection */
class T_Group:public L_Data_CallBack, public DroppableQueueInterface, public LoggableObjectInterface
{
  /** Layer 3 interface */
  Layer3 *layer3;
  /** output queue */
    Queue < GroupComm > outqueue;
    /** semaphore for output queue */
  pth_sem_t sem;
  /** group address */
  eibaddr_t groupaddr;
  bool init_ok;
  EIB_Priority sendprio;
  const static char outdropmsg[], indropmsg[];

public:
  T_Group (Layer3 * l3, Logs * t, eibaddr_t dest, int write_only, int inquemaxlen, int outquemaxlen, int peerquemaxlen);
  T_Group (Layer3 * l3, eibaddr_t dest, int write_only, EIB_Priority prio,
	   Logs * t, int inquemaxlen, int outquemaxlen, int peerquemaxlen);
    virtual ~ T_Group ();
  bool init ();

  void Get_L_Data (L_Data_PDU * l);

  /** receives APDU of a group telegram; aborts with NULL if stop occurs */
  GroupComm *Get (pth_event_t stop);
  /** send APDU c */
  void Send (const CArray & c);
};

/** Layer 4 raw individual connection */
class T_TPDU:public L_Data_CallBack, public DroppableQueueInterface,  public LoggableObjectInterface
{
  /** Layer 3 interface */
  Layer3 *layer3;
  /** output queue */
    Queue < TpduComm > outqueue;
    /** semaphore for output queue */
  pth_sem_t sem;
  /** source address to use */
  eibaddr_t src;

  bool init_ok;
  const static char outdropmsg[], indropmsg[];

public:
  T_TPDU (Layer3 * l3, Logs * t, eibaddr_t src,
	   int inquemaxlen, int outquemaxlen, int peerquemaxlen);
  virtual ~ T_TPDU ();
  bool init();

  void Get_L_Data (L_Data_PDU * l);

  /** receives TPDU of a telegram; aborts with NULL if stop occurs */
  TpduComm *Get (pth_event_t stop);
  /** send APDU c */
  void Send (const TpduComm & c);
};

/** Layer 4 T_Individual connection */
class T_Individual:public L_Data_CallBack, public DroppableQueueInterface,  public LoggableObjectInterface
{
  /** Layer 3 interface */
  Layer3 *layer3;
  /** output queue */
    Queue < CArray > outqueue;
    /** semaphore for output queue */
  pth_sem_t sem;
  /** destination address */
  eibaddr_t dest;

  bool init_ok;
  const static char outdropmsg[], indropmsg[];

public:
    T_Individual (Layer3 * l3,
		  Logs * t,
		  eibaddr_t dest, int write_only,
		  int inquemaxlen, int outquemaxlen, int peerquemaxlen);
    virtual ~ T_Individual ();
  bool init ();

  void Get_L_Data (L_Data_PDU * l);

  /** receives APDU of a telegram; aborts with NULL if stop occurs */
  CArray *Get (pth_event_t stop);
  /** send APDU c */
  void Send (const CArray & c);
};

/** implement a client T_Connection */
class T_Connection:public L_Data_CallBack, private Thread, public DroppableQueueInterface
{
  /** Layer 3 interface */
  Layer3 *layer3;
  /** input queue */
    Queue < CArray > in;
    /** buffer queue for layer 3 */
    Queue < L_Data_PDU * >buf;
  /** output queue */
    Queue < CArray > out;
    /** receiving sequence number */
  int recvno;
  /** sending sequence number */
  int sendno;
  /** state */
  int mode;
  /** repeat count of the transmitting frame */
  int repcount;
  eibaddr_t dest;
  /** semaphore for input queue */
  pth_sem_t insem;
  /** semaphre for buffer queue */
  pth_sem_t bufsem;
    /** semaphore for output queue */
  pth_sem_t outsem;
  bool init_ok;

  /** sends T_Connect */
  void SendConnect ();
  /** sends T_Disconnect */
  void SendDisconnect ();
  /** send T_ACK */
  void SendAck (int serno);
  /** Sends T_DataConnected */
  void SendData (int serno, const CArray & c);
  void Run (pth_sem_t * stop);

  const static char outdropmsg[], indropmsg[];

public:
  T_Connection (Layer3 * l3, Logs * t, eibaddr_t dest,
		   int inquemaxlen, int outquemaxlen, int peerquemaxlen);
   ~T_Connection ();

  bool init ();
  void Get_L_Data (L_Data_PDU * l);

  /** receives APDU of a telegram; aborts with NULL if stop occurs */
  CArray *Get (pth_event_t stop);
  /** send APDU c */
  void Send (const CArray & c);
};

#endif
