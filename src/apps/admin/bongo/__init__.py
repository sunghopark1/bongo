# THIS FILE SHOULD NOT BE INSTALLED!

# It exists only to merge the src/[...]/bongoadmin/bongo and src/libs/python/bongo
# directories in the Bongo source tree together.  In an installed
# system, they're going to be in the same place.

__path__.append(__path__[0] + "/../../../libs/python/bongo")
