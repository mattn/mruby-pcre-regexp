
# Constant
assert("PcreRegexp::CONSTANT") do
  PcreRegexp::IGNORECASE == 1 and PcreRegexp::EXTENDED == 2 and PcreRegexp::MULTILINE == 4
end


# Class method
assert("PcreRegexp.new") do
  PcreRegexp.new(".*") and PcreRegexp.new(".*", PcreRegexp::MULTILINE)
end

# Instance method
assert("PcreRegexp#==") do
  reg1 = reg2 = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  reg3 = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  reg4 = PcreRegexp.new("(https://[^/]+)[-a-zA-Z0-9./]+")

  reg1 == reg2 and reg1 == reg3 and !(reg1 == reg4)
end

assert("PcreRegexp#===") do
  reg = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  (reg === "http://example.com") == true and (reg === "htt://example.com") == false
end

# TODO =~

assert("PcreRegexp#casefold?") do
  reg1 = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", PcreRegexp::MULTILINE)
  reg2 = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", PcreRegexp::IGNORECASE | PcreRegexp::EXTENDED)
  reg3 = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", PcreRegexp::MULTILINE | PcreRegexp::IGNORECASE)
  reg4 = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  reg5 = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+", true)

  reg1.casefold? == false and reg2.casefold? == true and reg3.casefold? == true and
    reg4.casefold? == false and reg5.casefold? == true
end

assert("PcreRegexp#match") do
  reg = PcreRegexp.new("(https?://[^/]+)[-a-zA-Z0-9./]+")
  reg.match("http://masamitsu-murase.12345/hoge.html") and
    reg.match("http:///masamitsu-murase.12345/hoge.html").nil?
end

assert("PcreRegexp#source") do
  str = "(https?://[^/]+)[-a-zA-Z0-9./]+"
  reg = PcreRegexp.new(str)

  reg.source == str
end

# Extended patterns.
assert("PcreRegexp#match (no flags)") do
  patterns = [
    [ PcreRegexp.new(".*"), "abcd\nefg", "abcd" ],
    [ PcreRegexp.new("^a."), "abcd\naefg", "ab" ],
    [ PcreRegexp.new("^b."), "bacd\naefg", "ba" ],
    [ PcreRegexp.new(".$"), "bacd\naefg", "g" ]
  ]

  patterns.all?{ |reg, str, result| reg.match(str)[0] == result }
end

assert("PcreRegexp#match (multiline)") do
  patterns = [
    [ PcreRegexp.new(".*", PcreRegexp::MULTILINE), "abcd\nefg", "abcd\nefg" ]
  ]

  patterns.all?{ |reg, str, result| reg.match(str)[0] == result }
end

assert("PcreRegexp#match (ignorecase)") do
  patterns = [
    [ PcreRegexp.new("aBcD", PcreRegexp::IGNORECASE|PcreRegexp::EXTENDED), "00AbcDef", "AbcD" ],
    [ PcreRegexp.new("0x[a-f]+", PcreRegexp::IGNORECASE|PcreRegexp::EXTENDED), "00XaBCdefG", "0XaBCdef" ],
    [ PcreRegexp.new("0x[^c-f]+", PcreRegexp::IGNORECASE|PcreRegexp::EXTENDED), "00XaBCdefG", "0XaB" ]
  ]

  patterns.all?{ |reg, str, result| reg.match(str)[0] == result }
end

