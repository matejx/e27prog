#!/usr/bin/python

# e27blank.py by Matej Kogovsek, matej@hamradio.si, 3.feb.2016
# requires pySerial, http://pyserial.sourceforge.net

import sys,serial

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

if len(sys.argv) < 2:
  print 'usage: e27blank.py serial_if'
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

  try:
    atcmd('AT+E27BLANK=8000', 'OK')
  except:
    print 'Device NOT blank.'
    sys.exit(1)

finally:
  ser.close()

print 'Device is blank.'
