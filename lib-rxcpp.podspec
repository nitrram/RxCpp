Pod::Spec.new do |s|
  s.name                    = 'lib-rxcpp'
  s.version                 = '2.0.0'
  s.summary                 = 'The Reactive Extensions for Native'
  s.description             = <<-DESC
                              The Reactive Extensions for Native (__RxCpp__) is a library for composing asynchronous and event-based programs using observable sequences and LINQ-style query operators in C++.
                              DESC
  s.homepage                = 'git@gitlab.kancelar.seznam.cz:mobile/lib-shared-cpp-updater.git'
  s.license                 = 'SZN'
  s.author					= 'Seznam.cz, a.s.'
  s.platform                = :ios, '8.0'
  s.source                  = { :git => 'https://gitlab.kancelar.seznam.cz/mobile/lib-rxcpp' }
  s.requires_arc            = true
  s.module_name				= 'RxCpp'
  s.header_dir              = 'RxCpp'
  s.header_mappings_dir     = './'
  s.public_header_files		= '**/*.hpp'
  s.source_files            = 'Rx/v2/src/rxcpp/**/*.hpp'
  s.library                 = 'c++'
  s.pod_target_xcconfig     = { 'GCC_PREPROCESSOR_DEFINITIONS' => '__APPLE__ NOMINMAX',
                                'VALID_ARCHS' => 'arm64 armv7 armv7s i386 x86_64',
                                'OTHER_CFLAGS' => '-lpthread -fexceptions -frtti' }
end
