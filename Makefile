CXX = g++
CXXFLAGS = -std=c++17 -pthread
SOURCES = main.cpp routes.cpp utils.cpp BookingManager.cpp Flight.cpp Passenger.cpp Booking.cpp
TARGET = server

# Сборка
build:
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)
	@echo "✅ Сборка завершена!"

# Запуск
run: build
	@echo "🚀 Запуск сервера..."
	@echo "📍 Открой: https://$(CODESPACE_NAME)-8080.app.github.dev"
	./$(TARGET)

# Очистка
clean:
	rm -f $(TARGET)
	@echo "🧹 Очищено"

# Помощь
help:
	@echo "Доступные команды:"
	@echo "  make build  - собрать проект"
	@echo "  make run    - собрать и запустить"
	@echo "  make clean  - удалить исполняемый файл"