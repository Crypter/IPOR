.PHONY: clean All

All:
	@echo "----------Building project:[ CryptoModem - Debug ]----------"
	@"$(MAKE)" -f  "CryptoModem.mk"
clean:
	@echo "----------Cleaning project:[ CryptoModem - Debug ]----------"
	@"$(MAKE)" -f  "CryptoModem.mk" clean
