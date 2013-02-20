# make sure an exception is thrown somewhere in a library (map over a Range does not work, because there is no Range.empty)
(1..3).map(_ => 3)
