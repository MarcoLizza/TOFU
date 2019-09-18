local Bank = require("tofu.graphics").Bank
local Canvas = require("tofu.graphics").Canvas
local Font = require("tofu.graphics").Font
local Input = require("tofu.events").Input
local Class = require("tofu.util").Class
local System = require("tofu.core").System

local Game = Class.define()

local AMOUNT = 16
local PALETTES = { "pico-8", "arne-16", "c64", "cga" }

--384 x 224 pixels

function Game:__ctor()
  Canvas.palette("pico-8")

  self.bank = Bank.new("assets/sheet.png", 8, 8)
  self.font = Font.default(0, 15)
  self.time = 0
  self.x_size = Canvas.width() / AMOUNT
  self.y_size = Canvas.height() / AMOUNT
  self.palette = 1
  self.scale_x = 1.0
  self.scale_y = -1.0
  self.mode = 0
end

function Game:input()
  if Input.is_key_pressed(Input.SELECT) then
    self.palette = (self.palette % #PALETTES) + 1
    Canvas.palette(PALETTES[self.palette])
  elseif Input.is_key_pressed(Input.START) then
    self.mode = (self.mode + 1) % 10
  elseif Input.is_key_pressed(Input.DOWN) then
    self.scale_y = 1.0
  elseif Input.is_key_pressed(Input.UP) then
    self.scale_y = -1.0
  elseif Input.is_key_pressed(Input.RIGHT) then
    self.scale_x = 1.0
  elseif Input.is_key_pressed(Input.UP) then
    self.scale_x = -1.0
  end
end

function Game:update(delta_time)
  self.time = self.time + delta_time
  local index = (math.floor(self.time * 0.2) % #PALETTES) + 1
  if self.palette ~= index then
    self.palette = index
    Canvas.palette(PALETTES[index])
  end
end

function Game:render(ratio)
  Canvas.clear()

  if self.mode == 0 then
    for i = 0, AMOUNT - 1 do
      local x = self.x_size * i
      for j = 0, AMOUNT - 1 do
        local index = (i + j) % 7
        local color = (i + j) % AMOUNT
        local y = (Canvas.height() - 8) * (math.sin(self.time * 1.5 + i * 0.250 + j * 0.125) + 1) * 0.5
        Canvas.shift(1, color)
        self.bank:blit(index, x, y)
      end
    end
  elseif self.mode == 1 then
    local scale = (math.cos(self.time) + 1) * 3
    self.bank:blit(0, Canvas.width() / 2, Canvas.height() / 2, scale, scale, self.time * 0.5, 0.5, 0.5)
  elseif self.mode == 2 then
    self.bank:blit(0, Canvas.width() / 2, Canvas.height() / 2, 10, 10, math.pi / 2 * 1, 0.5, 0.5)
  elseif self.mode == 3 then
    self.bank:blit(0, Canvas.width() / 2, Canvas.height() / 2, 10, 10, math.pi / 4 * 1, 0.5, 0.5)
  elseif self.mode == 4 then
    local x = (Canvas.width() + 8) * (math.cos(self.time * 2.5) + 1) * 0.5
    local y = (Canvas.height() + 8) * (math.sin(self.time * 1.75) + 1) * 0.5
    self.bank:blit(0, x - 4, y - 4)
  elseif self.mode == 5 then
    local x = Canvas.width() * 0.25
    local y = Canvas.height() * 0.25
    self.bank:blit(1, x, y, self.scale_x * 8.0, self.scale_y * 8.0)
  end

  self.font:write(string.format("FPS: %d", System.fps()), 0, 0, "left")
  self.font:write(string.format("mode: %d", self.mode), Canvas.width(), 0, "right")
end

return Game
