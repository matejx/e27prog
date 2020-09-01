#!/usr/bin/python3

# e27rd.py by Matej Kogovsek, matej@hamradio.si, 3.feb.2016
# requires pySerial, http://pyserial.sourceforge.net

import sys,serial

def atcmd(cmnd, resp, to = 0.2):
  if ser.timeout != to:
    ser.timeout = to
  if len(cmnd) > 0:
    ser.flushInput()
    ser.write((cmnd + '\n').encode('ascii'))
  r = ser.readline().decode('ascii').rstrip()
  if len(resp) > 0 and r.find(resp) == -1:
    if r == '': r = '(none)'
    raise Exception('Error! expected ' + resp + '\ncmnd was: ' + cmnd + '\nresp was: ' + r + '\n')
  return r

if len(sys.argv) < 4:
  print('usage: e27rd.py serial_if filename size')
  sys.exit(1)

fs = int(sys.argv[3])

ser = serial.Serial(sys.argv[1], 115200)
try:
  for i in range(5):
    try:
      atcmd('AT+BUFRDDISP=1', 'OK')
      break
    except:
      pass
  if i == 4:
    print('programmer not responding')
    sys.exit(1)

  f = open(sys.argv[2], 'wb')
  try:
    a = 0
    while a < fs:
      l = min(fs-a, 64)
      s = atcmd('AT+E27RD='+hex(a)[2:].zfill(4)+','+str(l), '')
      atcmd('', 'OK')
      a = a + l
      f.write(bytes.fromhex(s))
      print(round(100*a/fs),'%')
  finally:
    f.close()

finally:
  ser.close()

print('Done.')
