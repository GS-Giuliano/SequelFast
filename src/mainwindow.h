#pragma once

#include <QAction>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMainWindow>
#include <QMap>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QProgressDialog>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QTextStream>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Alterna entre temas claro e escuro da aplicação
     * Carrega e aplica folhas de estilo (QSS) diferentes conforme o tema selecionado
     */
    void changeTheme();

    /**
     * @brief Abre diálogo para criação de novo banco de dados/schema
     * @param parent Widget pai para o diálogo modal
     */
    void createDatabaseDialog(QWidget* parent);

    /**
     * @brief Abre diálogo para criação de nova tabela no banco de dados
     * @param parent Widget pai para o diálogo modal
     */
    void createTableDialog(QWidget* parent);

    /**
     * @brief Estabelece conexão com o servidor MySQL/MariaDB especificado
     * @param selectedHost Nome da conexão configurada para conectar
     * @return true se a conexão foi estabelecida com sucesso, false caso contrário
     */
    bool host_connect(QString selectedHost);

    /**
     * @brief Atualiza a lista de conexões disponíveis na interface
     * Recarrega as conexões salvas do arquivo de configuração
     */
    void refresh_connections();

    /**
     * @brief Atualiza a lista de schemas/bancos de dados para uma conexão
     * @param selectedHost Nome da conexão ativa
     * @param jumpToTables Se true, automaticamente carrega as tabelas do primeiro schema
     */
    void refresh_schemas(QString selectedHost, bool jumpToTables);

    /**
     * @brief Atualiza informações específicas de um schema selecionado
     * @param selectedSchema Nome do schema a ser atualizado
     */
    void refresh_schema(QString selectedSchema);

    /**
     * @brief Atualiza a lista de tabelas do schema atualmente selecionado
     * @param selectedHost Nome da conexão ativa para buscar as tabelas
     */
    void refresh_tables(QString selectedHost);

    /**
     * @brief Atualiza a lista de consultas SQL favoritas salvas
     * Recarrega os favoritos do arquivo de configuração
     */
    void refresh_favorites();

    /**
     * @brief Inicia conexão SSH para tunneling seguro
     * @param selectedHost Referência para o nome da conexão que usará SSH (pode ser modificado)
     */
    void startSSH(QString& selectedHost);

    /**
     * @brief Encerra conexão SSH ativa
     * @param selectedHost Referência para o nome da conexão com SSH a ser encerrada
     */
    void endSSH(QString& selectedHost);

    /**
     * @brief Exibe caixa de diálogo de alerta personalizada
     * @param title Título da janela de alerta
     * @param message Mensagem a ser exibida no alerta
     */
    void customAlert(QString title, QString message);

    /**
     * @brief Abre consulta favorita selecionada no editor SQL
     * @param index Índice do item selecionado na lista de favoritos
     * @param run Se true, executa a consulta automaticamente após abrir
     */
    void open_selected_favorite(const QModelIndex& index, const bool& run);

    /**
     * @brief Inicia processo de backup de banco de dados
     * @param bkp_host Nome da conexão de onde fazer backup
     * @param bkp_schema Nome do schema a ser copiado
     * @param parent Widget pai para diálogos de progresso
     */
    void backup(const QString& bkp_host, const QString& bkp_schema, QWidget* parent);

    /**
     * @brief Inicia processo de restauração de backup de banco de dados
     * @param bkp_host Nome da conexão onde restaurar
     * @param bkp_schema Nome do schema de destino
     * @param parent Widget pai para diálogos de progresso
     */
    void restore(const QString& bkp_host, const QString& bkp_schema, QWidget* parent);

    /**
     * @brief Construtor da janela principal da aplicação
     * @param parent Widget pai (normalmente nullptr para janela principal)
     */
    MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destrutor da janela principal
     * Libera recursos e fecha conexões ativas
     */
    ~MainWindow();

private slots:

    /** @brief Slot ativado ao clicar no botão "Nova Conexão" */
    void on_buttonNewConns_clicked();

    /** @brief Slot ativado ao clicar no botão de filtro de schemas */
    void on_buttonFilterSchemas_clicked();

    /** @brief Slot ativado ao clicar no botão "Editar Conexões" */
    void on_buttonEditConns_clicked();

    /** @brief Slot ativado ao clicar no botão "Editar Tabelas" */
    void on_buttonEditTables_clicked();

    /** @brief Slot ativado ao clicar no botão "Editar Schemas" */
    void on_buttonEditSchemas_clicked();

    /** @brief Slot ativado ao clicar no botão de filtro de tabelas */
    void on_buttonFilterTables_clicked();

    /** @brief Slot ativado ao clicar no botão "Atualizar Schemas" */
    void on_buttonUpdateSchemas_clicked();

    /** @brief Slot ativado ao clicar no botão "Atualizar Tabelas" */
    void on_buttonUpdateTables_clicked();

    /**
     * @brief Slot ativado quando a aba atual da caixa de ferramentas esquerda muda
     * @param index Índice da nova aba selecionada
     */
    void on_toolBoxLeft_currentChanged(int index);

    /** @brief Slot ativado pela ação "Sair" do menu */
    void on_actionQuit_triggered();

    /** @brief Slot ativado pela ação "Nova Conexão" do menu */
    void on_actionNew_connection_triggered();

    /** @brief Slot ativado pela ação "Organizar em Mosaico" do menu */
    void on_actionTile_triggered();

    /** @brief Slot ativado pela ação "Organizar em Cascata" do menu */
    void on_actionCascade_triggered();

    /**
     * @brief Slot ativado ao clicar em um item da lista de conexões
     * @param index Índice do item clicado na lista de conexões
     */
    void on_listViewConns_clicked(const QModelIndex& index);

    /**
     * @brief Slot ativado ao clicar em um item da lista de schemas
     * @param index Índice do item clicado na lista de schemas
     */
    void on_listViewSchemas_clicked(const QModelIndex& index);

    /**
     * @brief Slot ativado ao clicar em um item da lista de tabelas
     * @param index Índice do item clicado na lista de tabelas
     */
    void on_listViewTables_clicked(const QModelIndex& index);

    /**
     * @brief Abre conexão selecionada da lista
     * @param index Índice da conexão a ser aberta
     */
    void listViewConns_open(const QModelIndex& index);

    /**
     * @brief Edita configurações da conexão selecionada
     * @param index Índice da conexão a ser editada
     */
    void listViewConns_edit(const QModelIndex& index);

    /**
     * @brief Clona/duplica a conexão selecionada
     * @param index Índice da conexão a ser clonada
     */
    void listViewConns_clone(const QModelIndex& index);

    /**
     * @brief Remove a conexão selecionada da lista
     * @param index Índice da conexão a ser removida
     */
    void listViewConns_remove(const QModelIndex& index);

    /**
     * @brief Exibe menu de contexto para lista de conexões
     * @param pos Posição do clique para exibir o menu
     */
    void show_context_menu_Conns(const QPoint& pos);

    /**
     * @brief Exibe menu de contexto para lista de schemas
     * @param pos Posição do clique para exibir o menu
     */
    void show_context_menu_Schemas(const QPoint& pos);

    /**
     * @brief Exibe menu de contexto para lista de tabelas
     * @param pos Posição do clique para exibir o menu
     */
    void show_context_menu_Tables(const QPoint& pos);

    /**
     * @brief Exibe menu de contexto para lista de favoritos
     * @param pos Posição do clique para exibir o menu
     */
    void show_context_menu_Favorites(const QPoint& pos);

    /**
     * @brief Abre tabela selecionada em nova janela SQL
     * @param index Índice da tabela a ser aberta
     */
    void handleListViewTables_open(const QModelIndex& index);

    /**
     * @brief Edita estrutura da tabela selecionada
     * @param index Índice da tabela a ser editada
     */
    void handleListViewTables_edit(const QModelIndex& index);

    /**
     * @brief Slot ativado ao duplo-clicar em um schema da lista
     * @param index Índice do schema clicado
     */
    void on_listViewSchemas_doubleClicked(const QModelIndex& index);

    /**
     * @brief Slot ativado ao duplo-clicar em uma conexão da lista
     * @param index Índice da conexão clicada
     */
    void on_listViewConns_doubleClicked(const QModelIndex& index);

    /**
     * @brief Slot ativado ao duplo-clicar em uma tabela da lista
     * @param index Índice da tabela clicada
     */
    void on_listViewTables_doubleClicked(const QModelIndex& index);

    /** @brief Slot ativado pela ação "Alterar Tema" do menu */
    void on_actionTheme_triggered();

    /** @brief Slot ativado pela ação "Novo Schema" do menu */
    void on_actionNew_schema_triggered();

    /** @brief Slot ativado pela ação "Nova Tabela" do menu */
    void on_actionNew_table_triggered();

    /** @brief Slot ativado pela ação "Gerenciar Usuários" do menu */
    void on_actionUsers_triggered();

    /** @brief Executa operações em lote (batch) configuradas */
    void batch_run();

    /** @brief Slot ativado ao clicar no botão "Editar Favoritos" */
    void on_buttonEditFavorites_clicked();

    /** @brief Slot ativado ao clicar no botão de filtro de favoritos */
    void on_buttonFilterFavorites_clicked();

    /**
     * @brief Slot ativado ao clicar em um item da lista de favoritos
     * @param index Índice do favorito clicado
     */
    void on_treeViewFavorites_clicked(const QModelIndex& index);

    /**
     * @brief Slot ativado ao duplo-clicar em um item da lista de favoritos
     * @param index Índice do favorito clicado
     */
    void on_treeViewFavorites_doubleClicked(const QModelIndex& index);

    /** @brief Slot ativado ao clicar no botão "Atualizar Favoritos" */
    void on_buttonUpdateFavorites_clicked();

    /** @brief Slot ativado pela ação "Backup" do menu */
    void on_actionBackup_triggered();

    /** @brief Slot ativado pela ação "Restaurar" do menu */
    void on_actionRestore_triggered();

    /** @brief Slot ativado pela ação "Estatisticas" do menu */
    void on_actionStatistics_triggered();

private:
    Ui::MainWindow* ui;

    QAction* action_db_options;
};

/**
 * @brief Delegate personalizado para destacar itens específicos em listas
 *
 * Esta classe herda de QStyledItemDelegate e permite destacar visualmente
 * um item específico da lista com cores diferenciadas (fundo amarelo, texto azul).
 */
class ColoredItemDelegate : public QStyledItemDelegate {
public:
    /**
     * @brief Construtor do delegate colorido
     * @param parent Objeto pai (normalmente a view que usará o delegate)
     * @param nomeDestacado Nome do item que deve ser destacado na lista
     */
    ColoredItemDelegate(QObject* parent = nullptr, const QString& nomeDestacado = "")
        : QStyledItemDelegate(parent), destaque(nomeDestacado) {
    }

    /**
     * @brief Método de pintura personalizado para destacar itens
     * @param painter Objeto QPainter para desenhar o item
     * @param option Opções de estilo para o item (posição, tamanho, etc.)
     * @param index Índice do modelo que está sendo pintado
     *
     * Se o texto do item corresponder ao nome destacado, aplica fundo amarelo
     * e texto azul. Caso contrário, usa a pintura padrão.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        if (index.data().toString() == destaque) {
            opt.backgroundBrush = QBrush(Qt::yellow);
            opt.palette.setColor(QPalette::Text, Qt::blue);
        }

        QStyledItemDelegate::paint(painter, opt, index);
    }

private:
    /** @brief Nome do item que deve ser destacado na lista */
    QString destaque;
};
