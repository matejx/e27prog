#!/usr/bin/python3

# e27blank.py by Matej Kogovsek, matej@hamradio.si, 3.feb.2016
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

if len(sys.argv) < 3:
  print('usage: e27blank.py serial_if size')
  sys.exit(1)

fs = int(sys.argv[2])

ser = serial.Serial(sys.argv[1], 115200)
try:
  for i in range(5):
    try:
      atcmd('AT+BUFRDDISP=0', 'OK')
      break
    except:
      pass
  if i == 4:
    print('programmer not responding')
    sys.exit(1)

  try:
    atcmd('AT+E27BLANK='+hex(fs)[2:].zfill(4), 'OK')
  except:
    print('Device NOT blank.')
    sys.exit(1)

finally:
  ser.close()

print('Device is blank.')
