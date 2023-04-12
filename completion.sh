_dry()
{
    local cur prev

    cur=${COMP_WORDS[COMP_CWORD]}
    prev=${COMP_WORDS[COMP_CWORD-1]}

    case ${COMP_CWORD} in
	1)
	    # Base-level completion: show subcommands
	    COMPREPLY=($(compgen -W "new list show init" -- ${cur}))
	    ;;
	2)
	    # Inner completion
	    case ${prev} in
		show)
		    COMPREPLY=($(compgen -C "ls ~/.dry/storage/damned/$(date '+%Y/%m/%d')" -- ${cur}))
		    COMPREPLY+=($(compgen -C "ls ~/.dry/storage/damned/$(date -d 'yesterday' '+%Y/%m/%d')" -- ${cur}))
		    #COMPREPLY+=("$(date -d 'today 0' "+%a %b %e %T %Z %Y")")
		    # mac: COMPREPLY+=("$(date -v 0H -v 0M -v 0S "+%a %b %e %T %Z %Y")")
		    #COMPREPLY+=("$(date -d 'yesterday 0' "+%a %b %e %T %Z %Y")")
		    # mac: COMPREPLY+=("$(date -v -1d -v 0H -v 0M -v 0S "+%a %b %e %T %Z %Y")")
		    ;;
		list)
		    COMPREPLY=($(compgen -W "yesterday today" -- ${cur}))
		    ;;
		new)
		    COMPREPLY=($(compgen -W "note video" -- ${cur}))
		    ;;
	    esac
	    ;;
	3)
	    case ${prev} in
		note|video|yesterday|today)
		    COMPREPLY=($(compgen -C "ls ~/.dry/storage/" -- ${cur}))
	    esac
	    ;;
	*)
	    # All other cases: provide no completion
	    COMPREPLY=()
	    ;;
    esac
}
complete -F _dry dry
