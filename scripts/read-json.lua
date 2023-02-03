local cj = require 'cjson'


function readAll(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content
end

local json = readAll('test.json')
local colors = cj.decode(json)

for i,v in pairs(colors) do
	for k,d in pairs(v) do
		print(k,d)
	end
end

