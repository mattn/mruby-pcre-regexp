MRuby::Gem::Specification.new('mruby-pcre-regexp') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mattn'
  
  case RUBY_PLATFORM
  when /mingw|mswin/
    spec.linker.libraries << ['pcre3', 'Shlwapi']
  else
    spec.linker.libraries << ['pcre']
  end
end
