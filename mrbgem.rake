MRuby::Gem::Specification.new('mruby-pcre-regexp') do |spec|
  spec.license = 'MIT'
  spec.authors = ['mattn', 'Tomasz Dabrowski']
  spec.summary = 'PCRE Regexp module for mruby'
  spec.version = '0.2.0'
  
  spec.linker.libraries << ['pcre']
end
