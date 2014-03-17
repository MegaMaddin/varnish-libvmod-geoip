%define         _varnish           varnish
%define         _varnishbuildver   3.0.5

Name:           varnish-libvmod-geoip
Version:        0.5
Release:        1%{?dist}

License:        BSD
Url:            http://github.com/MegaMaddin/%{name}
Group:          System Environment/Daemons
Summary:        Varnish VMOD for using GeoIP

Source0:        %{name}-%{version}.tar.gz
Source1:        http://repo.varnish-cache.org/source/%{_varnish}-%{_varnishbuildver}.tar.gz

# libvmod-geoip build requirements
BuildRequires:  GeoIP-devel GeoIP %{?el5:GeoIP-data}
# Varnish build requirements
BuildRequires:  pcre-devel automake libtool pkgconfig ncurses-devel libxslt groff readline-devel

Requires:       %{_varnish} = %{_varnishbuildver}
Requires:       %{_varnish}-libs = %{_varnishbuildver}
Requires:       GeoIP %{?el5:GeoIP-data}

BuildArch:      x86_64
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}

%description
This Varnish-Module enhances the varnish caching server to use the
Geo-IP functionality inside VCL-Code.

%prep
%setup -n %{name}

# to build a VMOD, we'll need the compiled varnish source tree
# so we have to build varnish first
%setup -n %{name} -D -T -a 1
cd %{_varnish}-%{_varnishbuildver}
%configure && %{__make} %{?_smp_mflags}
cd ..

%build
./autogen.sh
%{__chmod} +x configure
export VARNISHSRC=%{_builddir}/%{name}/%{_varnish}-%{_varnishbuildver}
export VMODDIR=%{_libdir}/varnish/vmods
%configure  
%{__make} %{?_smp_mflags}

# adopted from varnish spec
%check

# different place for GeoIP data files
%if 0%{?el6}
    %{__rm} src/tests/el5-test01.vtc
%elseif 0%{?el5}
    %{__rm} src/tests/el6-test01.vtc
%endif

%{__make} check %{?el5:abs_top_builddir='$(VARNISHSRC)/$(top_builddir)/'}

%install
[ %{buildroot} != "/" ] && %{__rm} -rf %{buildroot}
%{__make} install DESTDIR=%{buildroot}

# adopted from varnish.spec
# None of these for fedora
find %{buildroot}/%{_libdir}/ -name '*.la' -exec rm -f {} ';'
find %{buildroot}/%{_libdir}/ -name '*.a' -exec rm -f {} ';'

%clean
[ %{buildroot} != "/" ] && %{__rm} -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/varnish/vmods/libvmod_geoip*
%{_mandir}/man3/vmod_geoip.3.gz

%changelog
* Mon Mar 17 2014 Martin Probst <github@megamaddin.org> 0.5-1
- added get_continent_code function to be almost feature complete with
  GeoIP Country Edition
- added tests for new feature
- restructured code for better readability

* Thu May 09 2013 Martin Probst <github@megamaddin.org> - 0.4-1
- Changed function-calls from unsupported IP args, to supported STRING args
- added manpage

* Wed Apr 24 2013 Martin Probst <github@megamaddin.org> 0.3-1
- initial rpm release for el5 distribution
