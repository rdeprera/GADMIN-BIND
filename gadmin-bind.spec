%define desktop_vendor magnus-swe@telia.com
%{!?_dist: %{expand: %%define dist rhfc12}}

Summary:	GADMIN-BIND -- A GTK+ administation tool for ISC BIND.
Name:		gadmin-bind
Version:	0.2.5
Release:	0.1.%{dist}.nr
License:	GPL
Group:		Applications/System
URL:		http://mange.dynalias.org/linux.html
Source0:	http://mange.dynalias.org/linux/%{name}/%{name}-%{version}.tar.gz
BuildRoot:	%{_builddir}/%{name}-%{version}-root
Provides:	gadmin-bind

%description
GADMIN-BIND is a fast and easy to use GTK+ administration tool for ISC BIND.

%prep
%setup -q
%configure
SYSINIT_START_CMD="chkconfig named on"
SYSINIT_STOP_CMD="chkconfig named off"

%build
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall INSTALL_USER=`id -un` INSTALL_GROUP=`id -gn`

# pam auth
install -d %{buildroot}%{_sysconfdir}/pam.d/
install -d %{buildroot}%{_sysconfdir}/security/console.apps
install -m 644 etc/pam.d/%{name} %{buildroot}%{_sysconfdir}/pam.d/%{name}
install -m 644 etc/security/console.apps/%{name} %{buildroot}%{_sysconfdir}/security/console.apps/%{name}

# desktop entry
install -d %{buildroot}%{_datadir}/applications
install -m 644 desktop/%{name}.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop

# docs, theres no need to have version, rpm is not fun.
install -d %{buildroot}%{_datadir}/doc/%{name}
install -m 644 README COPYING AUTHORS ChangeLog %{buildroot}%{_datadir}/doc/%{name}/

%find_lang %name

%post 
if test ! -h %{_bindir}/gadmin-bind; then \
ln -s %{_bindir}/consolehelper %{_bindir}/gadmin-bind ; \
fi;

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{name}.lang

##%files
%defattr(0755, root, root)
%{_sbindir}/%{name}

%defattr(0644, root, root)
%{_sysconfdir}/pam.d/%{name}
%{_sysconfdir}/security/console.apps/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/*.png
%dir %{_datadir}/pixmaps/%{name}
%{_datadir}/pixmaps/%{name}/*.png

%dir %{_datadir}/doc/%{name}
%{_datadir}/doc/%{name}/README
%{_datadir}/doc/%{name}/COPYING
%{_datadir}/doc/%{name}/AUTHORS
%{_datadir}/doc/%{name}/ChangeLog

%changelog
* Fri Mar 19 2010 Magnus-swe <magnus-swe@telia.com>
- Fixes specfile desktop entry.
- Use correct sysinit satop command.
* Thu Feb 28 2008 Magnus-swe <magnus-swe@telia.com>
- Updated license for ip_handling.[c,h].
- Big thanks to Daniel Baumann!!!
