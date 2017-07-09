#!/usr/bin/python

# e27wr.py by Matej Kogovsek, matej@hamradio.si, 3.feb.2016
# requires pySerial, http://pyserial.sourceforge.net

import sys,serial,os,binascii

def atcmd(cmnd, resp, to = 0.2):
  if ser.timeout != to:
    ser.timeout = to
  if len(cmnd) > 0:
    ser.flushInput()
    ser.write(cmnd + '\n')
  r = ser.readline().rstrip();
  if len(resp) > 0 and r.find(resp) == -1:
    if r == '': r = '(none)'
    raise Exception('Error! expected ' + resp + '\ncmnd was: ' + cmnd + '\nresp was: ' + r + '\n')
  return r

if len(sys.argv) < 3:
  print 'usage: e27wr.py serial_if filename'
  sys.exit(1)

if os.path.getsize(sys.argv[2]) != 0x8000:
  print 'file size should be 32kB'
  sys.exit(1)

ser = serial.Serial(sys.argv[1], 115200)
try:
  for i in range(5):
    try:
      atcmd('AT+BUFRDDISP=0', 'OK')
      break
    except:
      pass
  if i == 4:
    print 'programmer not responding'
    sys.exit(1)

  f = open(sys.argv[2], 'rb')
  try:
    a = 0
    l = 64
    while a < 0x8000:
      d = f.read(l)
      atcmd('AT+BUFWR='+binascii.hexlify(d), 'OK')
      atcmd('AT+E27WR='+hex(a)[2:].zfill(4), 'OK')
      a = a + l
      print 100*a/0x8000,'%'
  finally:
    f.close()

finally:
  ser.close()

print 'Done.'
