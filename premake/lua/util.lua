local function GenOrderedIndex(t)
    local orderedIndex = {}
    for key in pairs(t) do
        table.insert( orderedIndex, key )
    end
    table.sort( orderedIndex )
    return orderedIndex
end

local function GetTableSize(t)
    local count = 0
    for _, _ in pairs(t) do
        count = count + 1
    end
    return count
end

local function OrderedNext(t, state)
    -- Equivalent of the next function, but returns the keys in the alphabetic
    -- order. We use a temporary ordered key table that is stored in the
    -- table being iterated.

    local key = nil
    if state == nil then
        -- the first time, generate the index
        t.__orderedIndex = GenOrderedIndex( t )
        key = t.__orderedIndex[1]
    else
        -- fetch the next value
        for i = 1, GetTableSize(t) do
            if t.__orderedIndex[i] == state then
                key = t.__orderedIndex[i+1]
            end
        end
    end

    if key then
        return key, t[key]
    end

    t.__orderedIndex = nil
    return
end

function OrderedPairs(t)
    -- Equivalent of the pairs() function on tables. Allows to iterate
    -- in order
    return OrderedNext, t, nil
end

function ToLower(str)
  return string.lower(str)
end

function Contains(table , key)
  if table == nil then
    return false
  end

  return table[key] ~= nil or table[ToLower(key)] ~= nil
end

function ContainsValue(table, value) 
  if table == nil then
    return false
  end

  for _, v in pairs(table) do
    if v == value then
      return true
    end
  end
end

function DirExists(dir)
  local ok, err, code = os.rename(dir, dir)
  if not ok then
    if code == 13 then
      -- Permission denied, but it exists
      return true
    end
  end
  return ok, err
end
