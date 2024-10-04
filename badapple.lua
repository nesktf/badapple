CHARSET = {
  [0] = ' ',
  [1] = '-',
  [2] = '=',
  [3] = "$"
}
WIDTH   = 60
HEIGHT  = 45
-- FRAME_C = 5258

local function sleep (a)
    local sec = tonumber(os.clock() + a)
    while (os.clock() < sec) do
    end
end

local f = io.open("./out0.bapple", "rb")

local byte_c  = 0
local row_c   = 0
local frame_c = 0
for byte in f:lines(1) do
  byte_c = byte_c + 1

  if (row_c == HEIGHT) then
    row_c = 0
    frame_c = frame_c + 1
    sleep(0.039)
    os.execute("clear")
  end

  local p1, p2, p3, p4
  for i = 1, 4 do
    local val = string.byte(byte)
    p1 = (val & 192) >> 6
    p2 = (val & 48) >> 4
    p3 = (val & 12) >> 2
    p4 = (val & 3)
  end
  io.write(
    CHARSET[p1]..CHARSET[p1]..CHARSET[p2]..CHARSET[p2]..
    CHARSET[p3]..CHARSET[p3]..CHARSET[p4]..CHARSET[p4]
  )

  if (byte_c == 15) then
    row_c = row_c + 1
    byte_c = 0
    print("")
  end
end

f:close()
