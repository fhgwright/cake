
### Class Macro: Create a generic macro file ##################################

# Macros are a bit different than those generated by fd2inline.
#
# Tag lists ("stdarg") are always initialized with the first tag value
# followed by __VA_ARGS__. This generates a compile-time error if no tags
# are supplied (TAG_DONE is the minimal tag list).

BEGIN {
    package Macro;

    sub new {
	my $proto  = shift;
	my %params = @_;
	my $class  = ref($proto) || $proto;
	my $self   = {};
	$self->{SFD}  = $params{'sfd'};
	$self->{BASE} = "${$self->{SFD}}{'BASENAME'}_BASE_NAME";
	$self->{BASE} =~ s/^([0-9])/_$1/;
	$self->{CALLBASE} = $self->{BASE};
	bless ($self, $class);
	return $self;
    }

    sub header {
	my $self = shift;
	my $sfd  = $self->{SFD};

	print "/* Automatically generated header! Do not edit! */\n";
	print "\n";
	print "#ifndef _INLINE_$$sfd{'BASENAME'}_H\n";
	print "#define _INLINE_$$sfd{'BASENAME'}_H\n";
	print "\n";
    }

    sub function {
	my $self      = shift;
	my %params    = @_;
	my $prototype = $params{'prototype'};
	my $sfd       = $self->{SFD};

	# Don't process private functions
	if ($prototype->{private}) {
	    return;
	}
	
	if ($$prototype{'type'} eq 'varargs') {
	    if ($prototype->{subtype} eq 'tagcall') {
		print "#ifndef NO_INLINE_STDARG\n";
	    }
	    else {
		print "#ifndef NO_INLINE_VARARGS\n";
	    }
	}
	
	$self->function_define (prototype => $prototype);
	$self->function_start (prototype => $prototype);
	for my $i (0 .. $$prototype{'numargs'} - 1 ) {
	    $self->function_arg (prototype => $prototype,
				 argtype   => $$prototype{'argtypes'}[$i],
				 argname   => $$prototype{'___argnames'}[$i],
				 argreg    => $$prototype{'regs'}[$i],
				 argnum    => $i );
	}
	$self->function_end (prototype => $prototype);

	if ($$prototype{'type'} eq 'varargs') {
	    if ($prototype->{subtype} eq 'tagcall') {
	    print "#endif /* !NO_INLINE_STDARG */\n";
	    }
	    else {
	    print "#endif /* !NO_INLINE_VARARGS */\n";
	    }
	}

	print "\n";
    }

    sub footer {
	my $self = shift;
	my $sfd  = $self->{SFD};

	print "#endif /* !_INLINE_$$sfd{'BASENAME'}_H */\n";
    }


    # Helper functions
    
    sub function_define {
	my $self      = shift;
	my %params    = @_;
	my $prototype = $params{'prototype'};
	my $sfd       = $self->{SFD};

	my $funcname  = $$prototype{'funcname'};
	
	my $argnames_ref  = $$prototype{'___argnames'};
	my $argnames      = join (', ', @{$argnames_ref});
	
	my $argnames2;
	my $argnames3 = join (', ', "___base", @{$argnames_ref});
	
	if ($$prototype{'type'} eq 'varargs') {
	    my $argnames_size = scalar(@{$argnames_ref}); 
	    $argnames2 = join (', ', $self->{CALLBASE}, @{$argnames_ref}[0..($argnames_size-2)], "__VA_ARGS__");
	}
	else {
	    $argnames2 = join (', ', $self->{CALLBASE}, @{$argnames_ref});;
	}
		
	print "#define $funcname($argnames) __${funcname}_WB($argnames2)\n";	
	print "#define __${funcname}_WB($argnames3) \\\n";
    }

    sub function_start {
	my $self      = shift;
	my %params    = @_;
	my $prototype = $params{'prototype'};
	my $sfd       = $self->{SFD};
	my $nr        = $$prototype{'return'} =~ /^(VOID|void)$/;

	if ($$prototype{'type'} eq 'varargs') {
	    if ($prototype->{subtype} eq 'tagcall' ||
		$prototype->{subtype} eq 'methodcall') {
		my $first_stdargnum = $$prototype{'numargs'} - 2;
		my $first_stdarg = $$prototype{'___argnames'}[$first_stdargnum];
	    
		printf "	({APTR _%s[] = { $first_stdarg, __VA_ARGS__ }; ",
		$prototype->{subtype} eq 'tagcall' ? "tags" : "message";
		print "__$$prototype{'real_funcname'}_WB((___base), ";
	    }
	    else {
		print "	({ULONG _args[] = { __VA_ARGS__ }; ";

		print "$$prototype{'real_funcname'}(";
	    }
	}
	elsif ($prototype->{type} eq 'cfunction') {
	    my $argtypes = join (', ',@{$$prototype{'argtypes'}});

	    if ($argtypes eq '') {
		if ($prototype->{nb}) {
		    $argtypes = "void";
		}
	    }
	    else {
		if (!$prototype->{nb}) {
		    $argtypes = "$sfd->{basetype}, $argtypes";
		}
	    }
	    
	    print "	({$$prototype{'return'} (*_func) ($argtypes) = \\\n";
	    print "	    ($$prototype{'return'} (*) ($argtypes))\\\n";

	    if ($$classes{'target'} eq 'morphos') {
		# Skip jmp instruction (is m68k ILLEGAL in MorphOS)
		my $o = $$prototype{'bias'} - 2;
		print "	    *((ULONG*) (((char*) $self->{BASE}) - $o));\\\n";
	    }
	    elsif ($classes->{target} eq 'aros') {
		my $o = $$prototype{'bias'} / 6;
		print "	    __AROS_GETVECADDR($self->{BASE}, $o);\\\n";
	    }
	    else {
		my $o = $$prototype{'bias'};
		print "	    (((char*) $self->{BASE}) - $o);\\\n";
	    }

	    print "	  (*_func)(";

	    if (!$prototype->{nb}) {
		print "($self->{BASE})";
		print ", " unless $prototype->{numargs} == 0;
	    }
	}
	else {
	    print STDERR "$prototype->{funcname}: Unhandled.\n";
	    die;
	}
    }

    sub function_arg {
	my $self      = shift;
	my %params    = @_;
	my $prototype = $params{'prototype'};
	my $argtype   = $params{'argtype'};
	my $argname   = $params{'argname'};
	my $argreg    = $params{'argreg'};
	my $argnum    = $params{'argnum'};
	my $sfd       = $self->{SFD};

	if ($$prototype{'type'} eq 'varargs') {
	    if ($prototype->{subtype} eq 'tagcall' ||
		$prototype->{subtype} eq 'methodcall') {
		my $first_stdargnum = $$prototype{'numargs'} - 2;

		# Skip the first stdarg completely
		if( $argnum != $first_stdargnum ) {
		    if ($argname eq '...') {
			if ($prototype->{subtype} eq 'tagcall') {
			    print "($argtype) _tags";
			}
			else {
			    print "($argtype) _message";
			}
		    }
		    else {
			print "($argname), ";
		    }
		}
	    }
	    else {
		if ($argname eq '...') {
		    print "($argtype) _args";
		}
		else {
		    print "($argname), ";
		}
	    }
	}
	elsif ($prototype->{type} eq 'cfunction') {
	    if ($argname eq '...' ) {
		print ($argnum != 0 ? ", __VA_ARGS__" : "__VA_ARGS__");
	    }
	    else {
		print ($argnum != 0 ? ", ($argname)" : "($argname)");
	    }
	}
	else {
	    print STDERR "$prototype->{funcname}: Unhandled.\n";
	    die;
	}
    }
    
    sub function_end {
	my $self      = shift;
	my %params    = @_;
	my $prototype = $params{'prototype'};
	my $sfd       = $self->{SFD};

	
	print "); })\n";
    }
}
