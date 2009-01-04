# Doxygen configurator.  Does its best to get a working build of doxygen
# wherever you are.  You should use this to make the developer docs, and
# then CPack to put it in the pakage -- CPack will copy destinations of 
# symlinks into the pakcer.  
# 
# In the case of the source package, then it's up to you to either 
# unconditionally install built docs or distributed docs, or some combination.
# 
# This file finds() all the necessary rubbish that doxygen needs.
#
# FUNCTIONS:
#
# add_doxygen(target_name installables_var template_file directives_list)
# 
#   Make ${target_name}, and ${target_name}_pdf,dvi,ps.  Generate rules which
#   will build the doxygen LaTeX stuff.  add_doxygen() attempts to depend
#   on everything that doxygen needs (eg additional header/footer files)
#   but if it didn't detect them all, touch the generated doxyfile, which is 
#   always depended on:
#   
#     ${CMAKE_BINARY_DIR}/${target_name}-doxyfile-generated
#   
#   WARNING: 
#     add_doxygen() forces some config options based on the dependancies
#     found and configuration directives.  (Eg USE_PDFLATEX if turned off if 
#     the pdflatex bin was not found).  However, in the interests of flexibility
#     I don't force all that many and (crucially) I don't check the template file
#     for changes.  This means that in the previous example, you could edit the 
#     additions file and the changes won't be noticed.
#     
#     The real meaning of this is that if the template file has an output turned
#     off then the install target for it will naturally be unable to work.  You
#     should therefore probably use the directives_list to force all options to do
#     with paths and directories.
#     
#     So in summary to the summary: it's a good idea to have a WANT_DOXYGEN_DOCS 
#     and WANT_PDF etc, use the directives_list to force the values which 
#     depend on them, and finally only generate install targets with respect to
#     the WANT_x variables.  If X is on, then force it in the conf and generate
#     the install target; otherwise force it off and don't generate the install.
#
#   The doxygen binary chooses the path for outputs.  Its working directory is the 
#   source dir, therefore if you wish to build to the binary directory (good idea) 
#   then you need OUTPUT_DIRECTORY to be an absolute path.  Other output directories
#   are are recommended be relative to this (although it *shouldn't* matter).
#   Same goes for using empty output directories (doxygen will put the output
#   in the main output dir).
#   
#   Look at ${CMAKE_BINARY_DIR}/${target_name}-doxyfile-forced for the options 
#   which have been forced: your directives_list is at the start of this, with 
#   add_doxygen()'s overrides after.
#   
#   installables_var
#     Name of a variable to return a list of installable things to.  This list
#     is relative to the main doxygen output directory; it is not a full path.
#   
#   target_name
#     The targets ${target_name}, ${target_name}_pdf, ${target_name}_dvi,
#     and ${target_name}_pdf are added.  Note that ${target_name} generates
#     everything.  The other targets call the makefile for the latex outputs.
#   
#     If you want to have the docs built as part of your ordinary build, depend
#     a target on one of these
#     
#   template_file
#     The basis of doxygen's configuration.  The resultant configuration is 
#     the template file  with the directives_list appended to it.
#     
#   directives_list
#     Things which will always be overridden.  The directives list is simply 
#     lines which go in a doxyfile:
#     
#       set(DIRECTIVES "OUTPUT_DIRECTORY = doc/" "INPUTS = include/")
#       
#     ${directives_list} does not need to contain anything, but it is normally useful
#     to make option()s which control things like GENERATE_LATEX.  Note though that
#     add_doxygen() might override them if it thinks they won't work.
#   
#   Warning: things may not function properly if doxygen builds output type
#   into the same directory.
#   
#   Note: be sure to put list argument in quotes!
#   
# doxygen_install_targets(doxygen_target, target)
#   Adds "make install" targets for generated doxygen files using the 
#   DOXYGEN_${target}_* variables and the assumption that a mirror of 
#   the doxygen build exists in the source dir.
#   
#   doxygen_target
#     A target that was set up using add_doxygen()
# 
#   want_rebuild
#     Should the buildsystem automatically (and always) build doxygen?
#     If want_rebuild, the install targets will point to the binarydir
#     (where add_doxygen outputs to).  If not want_rebuild, each file
#     is looked for in srcdir.  If it exists, then an install target is
#     added for it.
#   
#   The install locations are share/doc/${CMAKE_PROJECT_NAME}/.  Directories
#   automatically have the useless stuff left over from doxygen stripped 
#   out.

find_package(Doxygen)
find_package(LATEX)

find_program(CAT_EXE cat type)
find_program(MAKE_EXECUTABLE make gmake nmake)


function(add_doxygen installables_var target_name template_file directives_list)
  if (NOT DOXYGEN_EXECUTABLE)
    message(STATUS "Ignoring doxygen targets due to no doxygen exe.")
  endif()

  message(STATUS "Adding doxygen target ${target_name} from template file ${template_file}.")
  message("Note: output paths and generators should not be changed in the template file without re-configuring")

  # TODO silly limitation -- need to cause the buildsystem to reconfigure if the base doxyfile changes

  if(NOT CAT_EXE) 
    message("Guessing the name of cat/type as `cat'.  If doxygen doesn't work, manually define all paths in the base doxyfile.")
    set(CAT_EXE cat)
  endif()
  
  set(doxygen_conf_file "${CMAKE_BINARY_DIR}/${target_name}-doxyfile-generated")
  set(additions_file "${CMAKE_BINARY_DIR}/${target_name}-doxyfile-forced")
  
  message("Doxygen forced vars are in ${additions_file}.")

  ## Detect paths and vars defined by the template file and the overrides ##
  set(conf_output_dir "doxygen")
  set(conf_generate_latex YES)
  set(conf_generate_html YES)
  set(conf_generate_man NO)
  
  set(conf_html_dir "html")
  set(conf_latex_dir "latex")
  set(conf_man_dir "man")
  set(conf_use_pdflatex YES)
  
  # Stuff to match (code written which depends on it):
  # - OUTPUT_DIRECTORY
  # - GENERATE_LATEX LATEX_OUTPUT USE_PDFLATEX
  # - GENERATE_HTML HTML_OUTPUT
  # - GENERATE_MAN MAN_OUTPUT
  #  
  # Confs relevant for installation:
  # - HTML_FILE_EXTENSION DOT_IMAGE_FORMAT 
  # 
  # add_doxygen doesn't have explicit support for this yet:
  # - GENERATE_RTF RTF_OUTPUT
  # - GENERATE_HTMLHELP CHM_FILE
  #   - GENERATE_CHI (means a seperate file comes with HTMLHELP)
  # - GENERATE_AUTOGEN_DEF 
  # - GENERATE_DOCSET (something to do with xcode `doxygen will generate a Makefile 
  #   in the HTML output directory'.  It has a make install)
  #   - maybe force this off since I don't know how it works.
  # - GENERATE_XML XML_OUTPUT
  # 
  # There are other things, like header files etc which we should depend
  # on: MSCGEN_PATH, RTF_STYLESHEET_FILE RTF_EXTENSIONS_FILE HTML_STYLESHEET
  # HTML_HEADER HTML_FOOTER LATEX_HEADER 
  # 
  # TODO: 
  #   it might be better to just require that all these parameters are forced... it
  #   will probably break install targets otherwise... the main tricky thing is that
  #   we turn stuff off that *might* have been on initially which is confusing...
  #   
  #   Solutions?
  #   - require all contentious things to be forced
  #   - detect all contentious things everywhere and error immediately if they are not 
  #     enforcable
  #     - best, but the user needs to have a way to work around it 
  #       - suggestion in docs that the maintainer adds a WANT_x doc?
  #     - hard work
  # 
  file(READ ${template_file} file)
  
  # TODO: seriously?  This  should at least be done in a function somehow.
  string(
    REGEX MATCH
    "[Oo][Uu][Tt][Pp][Uu][Tt]_[Dd][Ii][Rr][Ee][Cc][Tt][Oo][Rr][Yy][^=]*=[\t\r ]*([^#]*).*" 
    "\\1" out "${file}"
  )
  
  string(
    REGEX MATCH
    "USE_PDFLATEX[^=]*=[\t\r ]*([^#]*).*" 
    "\\1" out "${file}"
  )
  
#   message("${out}")
  
  ## Generate the overrides file (also detect some configs in it) ##
  
  file(WRITE ${additions_file} "")
  foreach(iter ${directives_list}) 
    # TODO: match all the required vars again
    
    if (${iter} MATCHES "[Oo][Uu][Tt][Pp][Uu][Tt]_[Dd][Ii][Rr][Ee][Cc][Tt][Oo][Rr][Yy]")
      string(
        REGEX REPLACE 
        "[^=]*=[\t\r ]*([^#]*).*" 
        "\\1" doxygen_output_dir "${iter}"
      )
    else() 
      message("not it: ${iter}")
    endif()
    
    file(APPEND ${additions_file} "${iter}\n")
  endforeach()

  # TODO: also check that if latex, latexdir set etc etc.  We allow a blank
  #   latexdir tho because doxygen will just put it in the maindir (check this)

  if (NOT doxygen_output_dir)
    message("No doxygen output dir given.  Doxygen targets can't be added.")
    return()
  endif()

  if (NOT conf_generate_html AND NOT conf_generate_man AND NOT conf_generate_latex)
    message(STATUS "I'm confused; doxygen doesn't seem to be set to generate anything.")
    return()
  endif()

  ## Config forces based on found programs ##

  if (NOT DOXYGEN_DOT_PATH)
    file(APPEND ${additions_file} "HAVE_DOT = no\n")
  else()
    file(APPEND ${additions_file} "DOT_PATH = ${DOXYGEN_DOT_PATH}\n")
  endif()
  file(APPEND ${additions_file} "LATEX_BATCHMODE = yes\n")
  
  if (conf_generate_latex) 
    if (LATEX_COMPILER AND PDFLATEX_COMPILER) 
      # Let the template file choose.  Vars are forced anyway because if they get
      # changed later we can't detect it.
      if (conf_use_pdflatex)
        file(APPEND ${additions_file} "USE_PDFLATEX = yes\n")
        file(APPEND ${additions_file} "LATEX_CMD_NAME = ${PDFLATEX_COMPILER}\n")
      else()
        file(APPEND ${additions_file} "USE_PDFLATEX = no\n")
        file(APPEND ${additions_file} "LATEX_CMD_NAME = ${LATEX_COMPILER}\n")
      endif()
    elseif(PDFLATEX_COMPILER)
      file(APPEND ${additions_file} "USE_PDFLATEX = yes\n")
      file(APPEND ${additions_file} "LATEX_CMD_NAME = ${PDFLATEX_COMPILER}\n")
    elseif(LATEX_COMPILER)
      file(APPEND ${additions_file} "USE_PDFLATEX = no\n")
      file(APPEND ${additions_file} "LATEX_CMD_NAME = ${LATEX_COMPILER}\n")
    else()
      message("Generate LaTeX was on but no compiler was found.")
      file(APPEND ${additions_file} "GENERATE_LATEX = no\n")
      set(conf_generate_latex NO)
    endif()
    
    # TODO: there's other stuff: if it doesn't have a grapghics converter we need to
    #   turn off *all* graphics.  I can't remember what teh graphics prog was anyway.
    #   This only applies for LaTeX generation!
  endif()
  
  ## Configure paths ##
  
  
  # TODO: only if doxygen_output_dir is relative, then prepend binary_dir to it.
  set(absolute_doxygen_path "${CMAKE_BINARY_DIR}/${conf_output_dir}")
  string(REGEX REPLACE "\\/[\\/]+" "/" absolute_doxygen_path ${absolute_doxygen_path})
  
  # TODO: how does doxygen deal with a complete path for, eg latex?  Is it allowed?  
  #   If so we must detect it.
  
  set(absolute_latex_path "${absolute_doxygen_path}/${conf_latex_dir}")
  set(absolute_html_path "${absolute_doxygen_path}/${conf_html_dir}")
  set(absolute_man_path "${absolute_doxygen_path}/${conf_man_dir}")
  
  set(pdf_output "${absolute_latex_path}/refman.pdf")
  set(ps_output "${absolute_latex_path}/refman.ps")
  set(dvi_output "${absolute_latex_path}/refman.dvi")
  
  set(main_outputs "${absolute_doxygen_path}")
  if (conf_generate_man) 
    set(main_outputs "${main_outputs} ${absolute_man_path}")
  endif()
  if (conf_generate_html)
    set(main_outputs "${main_outputs} ${absolute_html_path}")
  endif()
  if (conf_generate_latex)
    set(main_outputs "${main_outputs} ${absolute_latex_path}")
  endif()
  
  
  message("** LATEX: ${absolute_latex_path}")
  message("** PDF:   ${pdf_output}")
 
  ## Add targets based on the paths ##
  
  # Applies to whatever the cwd is .
  set_property(
    DIRECTORY APPEND 
    PROPERTY "ADDITIONAL_MAKE_CLEAN_FILES" 
    ${additions_file} ${doxygen_conf_file} ${absolute_doxygen_path}
  )
  
  add_custom_command(
    OUTPUT ${doxygen_conf_file}
    COMMAND ${CAT_EXE} ${template_file} > ${doxygen_conf_file}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CAT_EXE} ${additions_file} >> ${doxygen_conf_file}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${template_file}
    COMMENT "Generating doxygen config file based on the template and forced values."
    VERBATIM
  )
  
  add_custom_command(
    OUTPUT ${absolute_doxygen_path} 
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxygen_conf_file}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${doxygen_conf_file}
    COMMENT "Generating main doxygen (html, .tex files ...)"
    VERBATIM 
  )
  
  # TODO dynamic paths; also if this works, then always use directories as the output.
  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/doxygen/html"
    DEPENDS ${absolute_doxygen_path} 
    COMMENT "Phony target to generate HTML."
    VERBATIM 
  )
  
  # TODO: is this really needed?  Also, make the path dynamic.  Also it's wrong
  #   anyway because the makefile might not get modified
  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/doxygen/latex/Makefile"
    DEPENDS ${absolute_doxygen_path} 
    COMMENT "Phony target to generate Makefile."
    VERBATIM 
  )
  
  add_custom_target(
    ${target_name}_main
    DEPENDS "${absolute_doxygen_path}"
    COMMENT "Generating HTML, LaTeX, man (if doxyfile changed)."
  )
  
  if (conf_generate_latex)
    if (NOT LATEX_COMPILER AND NOT PDFLATEX_COMPILER) 
      message(STATUS "No LaTeX found.  Doxygen LaTeX-based targets not added.")
    else()
      message(STATUS "Adding doxygen LaTeX targets.")
      
      if (NOT MAKE_EXECUTABLE)
        message("Make exe can't be found: doxygen LaTeX is not buildable.")
      else()
        # TODO: only the pdf command works!  Could it be an issue with USE_PDFLATEX?
        #   I think not.  It's prolly because pdflatex directly outputs the pdf and
        #   the others only output a dvi which can be converted either way.  Otherwise
        #   I can use ps2pdf... I need to test exactly what is outputted when not 
        #   pdflatex.
        #   
        add_custom_command(
          OUTPUT ${dvi_output}
          COMMAND ${MAKE_EXECUTABLE} dvi
          WORKING_DIRECTORY "${absolute_latex_path}"
          DEPENDS "doxygen/latex/Makefile"
          COMMENT "Calling doxygen's generated makefile for dvi (this is error prone!)."
          VERBATIM 
        )
      
        add_custom_command(
          OUTPUT ${pdf_output}
          COMMAND ${MAKE_EXECUTABLE} pdf
          WORKING_DIRECTORY "${absolute_latex_path}"
          DEPENDS "${CMAKE_BINARY_DIR}/doxygen/latex/Makefile"
          COMMENT "Calling doxygen's generated makefile for pdf (this is error prone!)."
          VERBATIM 
        )
        
        add_custom_command(
          OUTPUT ${ps_output}
          COMMAND ${MAKE_EXECUTABLE} ps
          WORKING_DIRECTORY "${absolute_latex_path}"
          DEPENDS "doxygen/latex/Makefile"
          COMMENT "Calling doxygen's generated makefile for ps (this is error prone!)."
          VERBATIM 
        )
  
        add_custom_target(
          ${target_name}_ps
          DEPENDS ${ps_output}
          COMMENT "Call ps makefile if ps isn't built."
        )
        
        add_custom_target(
          ${target_name}_dvi
          DEPENDS ${dvi_output}
          COMMENT "Call dvi makefile if dvi isn't built."
        )
        
        add_custom_target(
          ${target_name}_pdf
          DEPENDS ${pdf_output}
          COMMENT "Call pdf makefile if pdf isn't built."
        )
      endif()
    endif()
  endif()
  
endfunction()

function(doxygen_install_targets target instalables want_rebuild)
  message("FIXME: this function doesn't work yet.")
 
  return()

  message(STATUS "Adding doxygen make install targets.")


         
endfunction()

