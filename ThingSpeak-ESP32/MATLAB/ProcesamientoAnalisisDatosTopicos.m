thresholdFactor = 2; % Factor para el umbral dinámico
maxDataLength = 30; % Longitud máxima de los arreglos de datos

% Conexión al broker MQTT
mqttClient = mqttclient('tcp://192.168.209.2:1883');

% Suscribirse a los tópicos con función de callback
subscribe(mqttClient, 'esp32/ejex', 'Callback', @messageReceived);
subscribe(mqttClient, 'esp32/ejey', 'Callback', @messageReceived);

disp('Iniciando monitoreo...');

% Mantener el script en ejecución
while true
    pause(1);
end

% Definir la función de callback al final del script
function messageReceived(topic, data)
    % Acceder a las variables del script principal
    persistent inclinacionX inclinacionY prevX prevY mqttClient thresholdFactor maxDataLength newDataX newDataY

    % Inicializar variables persistentes en la primera llamada
    if isempty(inclinacionX)
        inclinacionX = [];
        inclinacionY = [];
        prevX = [];
        prevY = [];
        thresholdFactor = 2; % Ajustado
        maxDataLength = 30;
        mqttClient = evalin('base', 'mqttClient');
        newDataX = false;
        newDataY = false;
    end

    % Convertir el topic y data a caracteres
    topic = char(topic);
    data = char(data);

    % Procesar datos del tópico
    if strcmp(topic, 'esp32/ejex')
        valX = str2double(data);
        if ~isnan(valX) && ~isinf(valX)
            inclinacionX(end+1) = valX;
            if numel(inclinacionX) > maxDataLength
                inclinacionX = inclinacionX(end - maxDataLength + 1:end);
            end
            newDataX = true;
        end
    elseif strcmp(topic, 'esp32/ejey')
        valY = str2double(data);
        if ~isnan(valY) && ~isinf(valY)
            inclinacionY(end+1) = valY;
            if numel(inclinacionY) > maxDataLength
                inclinacionY = inclinacionY(end - maxDataLength + 1:end);
            end
            newDataY = true;
        end
    end

    % Procesar si hay nuevos datos de ambos ejes
    if newDataX && newDataY
        newDataX = false;
        newDataY = false;

        if numel(inclinacionX) >= 3 && numel(inclinacionY) >= 3
            % Usar los datos más recientes
            recentX = inclinacionX(end - min(numel(inclinacionX), maxDataLength) + 1:end);
            recentY = inclinacionY(end - min(numel(inclinacionY), maxDataLength) + 1:end);

            % Aplicar filtro promedio móvil
            windowSize = 2; % Ajustado
            inclinacionX_smooth = movmean(recentX, windowSize);
            inclinacionY_smooth = movmean(recentY, windowSize);

            % Calcular umbral dinámico
            stdX = std(inclinacionX_smooth);
            stdY = std(inclinacionY_smooth);
            dynamicThresholdX = thresholdFactor * stdX;
            dynamicThresholdY = thresholdFactor * stdY;

            % Obtener valores actuales suavizados
            currentX = inclinacionX_smooth(end);
            currentY = inclinacionY_smooth(end);

            % Inicializar valores previos si están vacíos
            if isempty(prevX)
                prevX = currentX;
                prevY = currentY;
            end

            % Calcular diferencias respecto a los valores previos
            diffX = abs(currentX - prevX);
            diffY = abs(currentY - prevY);

            % Mostrar información detallada para depuración
            disp(['currentX: ', num2str(currentX), ', prevX: ', num2str(prevX), ', diffX: ', num2str(diffX), ', thresholdX: ', num2str(dynamicThresholdX)]);
            disp(['currentY: ', num2str(currentY), ', prevY: ', num2str(prevY), ', diffY: ', num2str(diffY), ', thresholdY: ', num2str(dynamicThresholdY)]);

            % Detectar cambio brusco
            if diffX > dynamicThresholdX || diffY > dynamicThresholdY
                disp('¡Cambio brusco detectado! Publicando ALERTA...');
                write(mqttClient, 'esp32/alert', 'BLOCK');
                
                % Mantener últimos datos en los arreglos
                inclinacionX = inclinacionX(end-4:end);
                inclinacionY = inclinacionY(end-4:end);
                prevX = [];
                prevY = [];
            else
                disp('No se detectó cambio brusco.');
            end

            % Actualizar valores previos
            prevX = currentX;
            prevY = currentY;
        else
            disp('No hay suficientes datos para procesar.');
        end
    end
end
