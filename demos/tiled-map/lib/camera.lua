--[[
  Copyright (c) 2019 Marco Lizza (marco.lizza@gmail.com)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
]]--

local Canvas = require("tofu.graphics").Canvas
local Class = require("tofu.util").Class

local function bound(x, y, aabb)
  return math.min(math.max(x, aabb.x0), aabb.x1), math.min(math.max(y, aabb.y0), aabb.y1)
end

local Camera = Class.define()

-- TODO: add camera scaling, useful to draw minimap.
function Camera:__ctor(id, grid, bank, columns, rows, screen_x, screen_y, anchor_x, anchor_y)
  self.id = id
  self.grid = grid
  self.bank = bank
  self.screen_x = screen_x or 0
  self.screen_y = screen_y or 0
  self.columns = columns
  self.rows = rows
  self.width = columns * self.bank:cell_width()
  self.height = rows * self.bank:cell_height()

  self:center_at(anchor_x or 0.5, anchor_y or 0.5)
end

function Camera:center_at(anchor_x, anchor_y)
  self.anchor_x = anchor_x
  self.anchor_y = anchor_y
  self.offset_x = math.tointeger(anchor_x * self.width) -- Always an integer offset
  self.offset_y = math.tointeger(anchor_y * self.height)
  self.aabb = {
      x0 = self.offset_x,
      y0 = self.offset_y,
      x1 = self.grid:width() * self.bank:cell_width() - (self.width - self.offset_x) - 1,
      y1 = self.grid:height() * self.bank:cell_height() - (self.height - self.offset_y) - 1
    }
  self:move_to(self.x or 0, self.y or 0)
end

function Camera:move_to(x, y)
  self.x, self.y = bound(x, y, self.aabb)

  local map_x = math.tointeger(self.x) - self.offset_x
  local map_y = math.tointeger(self.y) - self.offset_y

  if self.map_x == map_x and self.map_y == map_y then
    return
  end
  self.map_x, self.map_y = map_x, map_y -- Track offsetted map position to track *real* changes.

  local cw, ch = self.bank:cell_width(), self.bank:cell_height()
  local start_column = math.tointeger(map_x / cw)
  local start_row = math.tointeger(map_y / ch)
  local column_offset = -(map_x % cw)
  local row_offset = -(map_y % ch)

  if self.start_column ~= start_column or self.start_row ~= start_row then
    self.start_column = start_column
    self.start_row = start_row
    self:prepare_()
  end
  self.column_offset = column_offset
  self.row_offset = row_offset
end

function Camera:to_screen(x, y)
  return x - self.x + self.offset_x + self.screen_x, y - self.y + self.offset_y + self.screen_y
end

function Camera:to_world(x, y)
  return x - self.screen_x + self.offset_x + self.x, y - self.screen_y + self.offset_y + self.y
end

function Camera:update(_)
  -- Override.
end

function Camera:pre_draw()
  -- Override.
end

function Camera:draw()
  Canvas.clipping(self.screen_x, self.screen_y, self.width, self.height)

  local ox, oy = self.screen_x + self.column_offset, self.screen_y + self.row_offset
  for _, v in ipairs(self.batch) do
    local cell_id, cell_x, cell_y = table.unpack(v)
    self.bank:blit(math.tointeger(cell_id), cell_x + ox, cell_y + oy)
  end
end

function Camera:post_draw()
  -- Override.
end

function Camera:prepare_()
  local cw, ch = self.bank:cell_width(), self.bank:cell_height()

  local rows = math.min(self.grid:width() - self.start_row, self.rows + 1) -- We handle an additional row/column
  local columns = math.min(self.grid:height() - self.start_column, self.columns + 1) -- for sub-tile scrolling

  local batch = {}
  local y = 0
  local r = self.start_row
  for _ = 1, rows do
    local x = 0
    local c = self.start_column
    for _ = 1, columns do
      local cell_id = self.grid:peek(c, r)
      table.insert(batch, { cell_id, x, y })
      x = x + cw
      c = c + 1
    end
    y = y + ch
    r = r + 1
  end
  self.batch = batch
end

function Camera:__tostring()
  return string.format("[%s] %.0f %0.f | %.0f %0.f | %0.f %0.f",
      self.id,
      self.x, self.y,
      self.start_column, self.start_row,
      self.column_offset, self.row_offset)
end

return Camera