#? replace(sub = "\t", by = " ")

proc todo*(msg: string) =
	stderr.write("TODO: ", msg, "\n")
