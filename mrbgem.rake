MRuby::Gem::Specification.new('mruby-pcre-regexp') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mattn'
  
  spec.linker.libraries << ['pcre']
end
