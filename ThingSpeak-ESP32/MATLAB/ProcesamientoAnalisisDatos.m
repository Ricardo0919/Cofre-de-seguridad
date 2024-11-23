thresholdFactor = 3; % Factor para el umbral dinámico
url = 'https://api.thingspeak.com/channels/2746417/feeds.json?api_key=09KFL869WPOHHVVH&results=100';

% Configuración MQTT
mqttClient = mqttclient('tcp://192.168.209.2:1883');

disp('Iniciando monitoreo...');

% Variables para almacenar el último valor
prevX = [];
prevY = [];

while true
    try
        % Leer datos de ThingSpeak
        data = webread(url);
        feeds = data.feeds;

        % Extraer y validar inclinaciones
        inclinacionX = [];
        inclinacionY = [];
        for i = 1:numel(feeds)
            valX = str2double(feeds(i).field1);
            valY = str2double(feeds(i).field2);
            if ~isnan(valX) && ~isinf(valX) && ~isnan(valY) && ~isinf(valY)
                inclinacionX(end+1) = valX;
                inclinacionY(end+1) = valY;
            end
        end

        % Verificar si hay suficientes datos
        if numel(inclinacionX) >= 5
            % Aplicar un filtro promedio móvil para suavizar la señal
            windowSize = 5;
            inclinacionX_smooth = movmean(inclinacionX, windowSize);
            inclinacionY_smooth = movmean(inclinacionY, windowSize);

            % Calcular la desviación estándar para el umbral dinámico
            stdX = std(inclinacionX_smooth);
            stdY = std(inclinacionY_smooth);
            dynamicThresholdX = thresholdFactor * stdX;
            dynamicThresholdY = thresholdFactor * stdY;

            % Obtener el último valor suavizado
            currentX = inclinacionX_smooth(end);
            currentY = inclinacionY_smooth(end);

            % Si es la primera iteración, inicializar previos
            if isempty(prevX)
                prevX = currentX;
                prevY = currentY;
            end

            % Calcular la diferencia respecto al valor previo
            diffX = abs(currentX - prevX);
            diffY = abs(currentY - prevY);

            % Actualizar los valores previos
            prevX = currentX;
            prevY = currentY;

            % Detectar cambio brusco
            if diffX > dynamicThresholdX || diffY > dynamicThresholdY
                disp('¡Cambio brusco detectado! Publicando ALERTA...');
                write(mqttClient, 'esp32/alert', 'BLOCK');
            else
                write(mqttClient, 'esp32/alert', 'NONE');
            end
        else
            disp('No hay suficientes datos para procesar.');
        end

        % Esperar antes de la siguiente lectura
        pause(5);
    catch ME
        disp(['Error durante el monitoreo: ', ME.message]);
    end
end
