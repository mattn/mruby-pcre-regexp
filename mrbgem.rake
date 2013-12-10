MRuby::Gem::Specification.new('mruby-pcre-regexp') do |spec|
  spec.license = 'MIT'
  spec.authors = 'mattn'
  
  case RUBY_PLATFORM
  when /mingw|mswin/
    spec.linker.libraries << ['pcre3', 'Shlwapi']
    spec.cc.include_paths += ["C:/Windows/system"]
    spec.linker.library_paths += ["C:/Windows/system"]
  else
    spec.linker.libraries << ['pcre']
  end
end
