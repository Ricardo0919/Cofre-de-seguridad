% Variables globales para compartir entre el script y el callback
global inclinacionX inclinacionY maxDataLength newDataX newDataY ...
       hPlotX_raw hPlotY_raw hPlotX_smooth hPlotY_smooth ...
       mqttClient isRunning hFig

maxDataLength = 100; % Ajusta el tamaño según tus necesidades

% Inicializar variables
inclinacionX = [];
inclinacionY = [];
newDataX = false;
newDataY = false;
isRunning = true;

% Conexión al broker MQTT
mqttClient = mqttclient('tcp://10.25.100.90:1883');

% Crear figura para graficar
hFig = figure(1);
set(hFig, 'CloseRequestFcn', @closeFigure); % Añadir función para manejar el cierre de la figura

% Subplot para Inclinación X en el tiempo
subplot(2,1,1);
hPlotX_raw = plot(NaN, NaN, 'b', 'DisplayName', 'Datos crudos');
hold on;
hPlotX_smooth = plot(NaN, NaN, 'r', 'DisplayName', 'Datos filtrados');
title('Inclinación X en el tiempo');
xlabel('Muestras');
ylabel('Valor');
legend('show');
hold off;

% Subplot para Inclinación Y en el tiempo
subplot(2,1,2);
hPlotY_raw = plot(NaN, NaN, 'b', 'DisplayName', 'Datos crudos');
hold on;
hPlotY_smooth = plot(NaN, NaN, 'r', 'DisplayName', 'Datos filtrados');
title('Inclinación Y en el tiempo');
xlabel('Muestras');
ylabel('Valor');
legend('show');
hold off;

% Suscribirse a los tópicos con función de callback
subscribe(mqttClient, 'esp32/ejex', 'Callback', @messageReceived);
subscribe(mqttClient, 'esp32/ejey', 'Callback', @messageReceived);

disp('Iniciando monitoreo...');

% Mantener el script en ejecución hasta que se cierre la figura
while ishandle(hFig) && isRunning
    pause(0.1);
    drawnow; % Permitir que MATLAB procese eventos
end

% Limpiar al finalizar
unsubscribe(mqttClient);
clear mqttClient;

disp('Monitoreo finalizado.');

% Función para manejar el cierre de la figura
function closeFigure(~, ~)
    global isRunning
    isRunning = false; % Cambiar bandera para detener el bucle
    closereq(); % Cerrar la figura correctamente
end

% Función de callback para recibir mensajes MQTT
function messageReceived(topic, data)
    try
        % Acceder a las variables globales
        global inclinacionX inclinacionY maxDataLength newDataX newDataY ...
               hPlotX_raw hPlotY_raw hPlotX_smooth hPlotY_smooth

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
                % disp(['Valor recibido en X: ', num2str(valX)]); % Mensaje opcional para depuración
            end
        elseif strcmp(topic, 'esp32/ejey')
            valY = str2double(data);
            if ~isnan(valY) && ~isinf(valY)
                inclinacionY(end+1) = valY;
                if numel(inclinacionY) > maxDataLength
                    inclinacionY = inclinacionY(end - maxDataLength + 1:end);
                end
                newDataY = true;
                % disp(['Valor recibido en Y: ', num2str(valY)]); % Mensaje opcional para depuración
            else
                % disp('Valor inválido recibido en Y.'); % Mensaje opcional para depuración
            end
        end

        % Procesar si hay nuevos datos de cualquiera de los ejes
        if newDataX || newDataY
            % Aplicar filtro promedio móvil si hay suficientes datos
            windowSize = 5; % Ajusta el tamaño de la ventana según tus necesidades

            if numel(inclinacionX) >= windowSize
                inclinacionX_smooth = movmean(inclinacionX, windowSize);
            else
                inclinacionX_smooth = inclinacionX;
            end

            if numel(inclinacionY) >= windowSize
                inclinacionY_smooth = movmean(inclinacionY, windowSize);
            else
                inclinacionY_smooth = inclinacionY;
            end

            % Actualizar gráficos en tiempo real
            % Actualizar datos crudos
            set(hPlotX_raw, 'XData', 1:numel(inclinacionX), 'YData', inclinacionX);
            set(hPlotY_raw, 'XData', 1:numel(inclinacionY), 'YData', inclinacionY);

            % Actualizar datos filtrados
            set(hPlotX_smooth, 'XData', 1:numel(inclinacionX_smooth), 'YData', inclinacionX_smooth);
            set(hPlotY_smooth, 'XData', 1:numel(inclinacionY_smooth), 'YData', inclinacionY_smooth);

            drawnow; % Actualizar figuras

            newDataX = false;
            newDataY = false;
        end
    catch ME
        disp(['Error en messageReceived: ', ME.message]);
    end
end
