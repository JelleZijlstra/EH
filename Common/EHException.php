<?php
class EHException extends Exception {
	const E_USER = 0x1;
	const E_RECOVERABLE = 0x2;
	const E_FATAL = 0x3;
	public function __construct($message, $code = 0, Exception $previous = NULL) {
		parent::__construct($message, $code, $previous);
		$this->handle();
	}
	public function handle() {
		switch($this->code) {
			case self::E_USER:
				return $this->handle_user();
			case self::E_RECOVERABLE:
				return $this->handle_recoverable();
			case self::E_FATAL:
				return $this->handle_fatal();
			default:
				throw new EHException(
					"Unknown exception code",
					self::E_FATAL,
					$this
				);
		}
		return;
	}
	private function handle_user() {
	// do nothing for now
		return;
	}
	private function handle_recoverable() {
	// do nothing for now
		return;
	}
	private function handle_fatal() {
		echo "EH fatal error: " . $this->getMessage() . " in " . $this->getFile() . " on line " . $this->getLine() . PHP_EOL;
		// allow cleanup, like saveifneeded(), with __destruct() methods
		exit(1);
	}
}
