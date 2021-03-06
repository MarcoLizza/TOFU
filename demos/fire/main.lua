local Grid = require("tofu.collections").Grid
local Canvas = require("tofu.graphics").Canvas
local Font = require("tofu.graphics").Font
local Input = require("tofu.events").Input
local Class = require("tofu.util").Class
local System = require("tofu.core").System

local STEPS = 64
local PALETTE = {
    0xFF000000, 0xFF240000, 0xFF480000, 0xFF6D0000,
    0xFF910000, 0xFFB60000, 0xFFDA0000, 0xFFFF0000,
    0xFFFF3F00, 0xFFFF7F00, 0xFFFFBF00, 0xFFFFFF00,
    0xFFFFFF3F, 0xFFFFFF7F, 0xFFFFFFBF, 0xFFFFFFFF
  }

local Main = Class.define()

function Main:__ctor()
  Canvas.palette(PALETTE)

  self.font = Font.default(0, 15)
  self.x_size = Canvas.width() / STEPS
  self.y_size = Canvas.height() / STEPS
  self.windy = false
  self.damping = 1.0
  self.grid = Grid.new(STEPS, STEPS, 0)

  self:reset()
end

function Main:reset()
  self.grid:fill(0)
  self.grid:stride(0, STEPS - 1, #PALETTE - 1, STEPS)
end

function Main:input()
  if Input.is_pressed(Input.SELECT) then
    self.windy = not self.windy
  elseif Input.is_pressed(Input.LEFT) then
    self.damping = self.damping - 0.1
  elseif Input.is_pressed(Input.RIGHT) then
    self.damping = self.damping + 0.1
  elseif Input.is_pressed(Input.START) then
    self:reset()
  end
end

function Main:update(_)
  local windy = self.windy
  self.grid:process(function(column, row, value)
      if row == 0 then -- Skip the 1st row entirely.
        return column, row, value
      end

      if value > 0 then
        local delta = math.random(0, 1) * self.damping
        value = value - delta
        if value < 0 then
          value = 0
        end
      end

      if windy then
        column = column + math.random(0, 3) - 1
        if column < 0 then
          column = 0
        elseif column > STEPS - 1 then
          column = STEPS - 1
        end
      end

      return column, row - 1, value -- Move up one row!
    end)
end

function Main:render(_)
  Canvas.clear()

  local w = self.x_size
  local h = self.y_size
  self.grid:scan(function(column, row, value)
      local x = column * w
      local y = row * h
      if value > 0 then
        Canvas.rectangle("fill", x, y, w, h, math.tointeger(value))
      end
    end)

    self.font:write(string.format("FPS: %d", System.fps()), 0, 0, "left")
    self.font:write(string.format("D: %.2f", self.damping), Canvas.width(), 0, "right")
end

return Main
