local std = stead

local instead = std.obj { nam = '@instead' }

instead.nosave = false
instead.noautosave = false

local iface = std '@iface'
local type = std.type

local dict = {}

local function get_bool(o, nam)
	if type(o[nam]) == 'boolean' then
		return o[nam]
	end
	if type(o[nam]) == 'function' then
		return o:nam()
	end
	return nil
end

iface.notitle = false

instead.get_title = std.cacheable('title', function()
	if get_bool(iface, 'notitle') then
		return
	end
	return std.titleof(stead.here())
end)

function iface:fading()
end

function instead.autosave(slot)
end

function instead.menu(n)
end

function instead.restart(v)
end

function iface:title(str) -- hide title
	return str
end

std.stat = std.class({
	__stat_type = true;
}, std.obj);

std.menu = std.class({
	__menu_type = true;
	new = function(self, v)
		if type(v) ~= 'table' then
			std.err ("Wrong argument to std.menu:"..std.tostr(v), 2)
		end
		v = std.obj(v)
		std.setmt(v, self)
		return v
	end;
	inv = function(s, ...)
		local r, v = std.call(s, 'act', ...)
		if r ~= nil then
			return r, v
		end
		return true, false -- menu mode
	end;
}, std.obj);
std.setmt(std.phr, std.menu) -- make phrases menus
std.setmt(std.ref '@', std.menu) -- make xact menu

function iface:xref(str, o, ...)
	if type(str) ~= 'string' then
		std.err ("Wrong parameter to iface:xref: "..std.tostr(str), 2)
	end
	if not std.is_obj(o) or std.is_obj(o, 'stat') then
		return str
	end
	local a = { ... }
	local args = ''
	for i = 1, #a do
		if type(a[i]) ~= 'string' and type(a[i]) ~= 'number' then
			std.err ("Wrong argument to iface:xref: "..std.tostr(a[i]), 2)
		end
		args = args .. ' '..std.dump(a[i])
	end
	local xref = std.string.format("%s%s", std.deref_str(o), args)
	-- std.string.format("%s%s", iface:esc(std.deref_str(o)), iface:esc(args))

	table.insert(dict, xref)
	xref = std.tostr(#dict)
	return str..std.string.format("(%s)", xref)
end

local function fmt_stub(self, str)
	return str
end

iface.em = fmt_stub
iface.center = fmt_stub
iface.just = fmt_stub
iface.left = fmt_stub
iface.right = fmt_stub
iface.bold = fmt_stub
iface.top = fmt_stub
iface.bottom = fmt_stub
iface.middle = fmt_stub
iface.nb = fmt_stub
iface.anchor = fmt_stub
iface.img = fmt_stub
iface.imgl = fmt_stub
iface.imgr = fmt_stub
iface.under = fmt_stub
iface.st = fmt_stub
iface.tab = fmt_stub
iface.y = fmt_stub

local iface_cmd = iface.cmd -- save old

function iface:cmd(inp)
	local a = std.split(inp)
	if std.tonum(a[1]) then
		std.table.insert(a, 1, 'act')
	end
	if a[1] == 'act' or a[1] == 'use' or a[1] == 'go' then
		if a[1] == 'use' then
			local use = std.split(a[2], ',')
			for i = 1, 2 do
				local u = std.tonum(use[i])
				if u then
					use[i] = dict[u]
				end
			end
			a[2] = std.join(use, ',')
		elseif std.tonum(a[2]) then
			a[2] = dict[std.tonum(a[2])]
		end
		inp = std.join(a)
	end
	return iface_cmd(self, inp)
end

std.obj { -- input object
	nam = '@input';
};

-- some aliases
menu = std.menu
stat = std.stat

-- fake sound
local sound = std.obj {
	nam = '@snd';
}

sound.set = function() end
sound.play = function() end
sound.stop = function() end
sound.music = function() end
sound.stop_music = function() end
sound.music_fading = function() end
sound.new = function() return sound end

-- fake timer
std.obj {
	nam = '@timer';
	get = function(s)
		return s.__timer or 0;
	end;
	stop = function(s)
		return s:set(0)
	end;
	set = function(s, v)
		s.__timer = v
		return true
	end;
}

-- fake sprite
std.obj {
	nam = '@sprite';
	new = function() end;
	fnt = function() end;
	scr = function() end;
	direct = function() return false end;
}
-- fake pixels
std.obj {
	nam = '@pixels';
	fnt = function() end;
	new = function() end;
}
-- fake themes
local theme = std.obj {
	nam = '@theme';
	{
		win = { gfx = {}};
		inv = { gfx = {}};
		menu = { gfx = {}};
		gfx = {};
		snd = {};
	};
}

function theme.restore()
end

function theme.set()
end

function theme.reset()
end

function theme.name()
end

function theme.get()
end

function theme.win.reset()
end

function theme.win.geom()
end

function theme.win.color()
end

function theme.win.font()
end

function theme.win.gfx.reset()
end

function theme.win.gfx.up()
end

function theme.win.gfx.down()
end

function theme.inv.reset()
end

function theme.inv.geom()
end

function theme.inv.color(f)
end

function theme.inv.font()
end

function theme.inv.mode()
end

function theme.inv.gfx.reset()
end

function theme.inv.gfx.up()
end

function theme.inv.gfx.down()
end

function theme.menu.reset()
end

function theme.menu.bw(w)
end

function theme.menu.color()
end

function theme.menu.font()
end

function theme.menu.gfx.reset()
end

function theme.menu.gfx.button()
end;

function theme.gfx.reset()
end

function theme.gfx.cursor()
end

function theme.gfx.mode()
end

function theme.gfx.pad()
end

function theme.gfx.bg()
end

function theme.snd.reset()
end

function theme.snd.click()
end


std.mod_init(function()
	std.rawset(_G, 'instead', instead)
end)
std.mod_start(function()
	dict = {}
end)
std.mod_step(function(state)
	if state then
		dict = {}
	end
end)
