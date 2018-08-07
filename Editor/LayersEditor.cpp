#include "LayersEditor.hpp"
#include "MainWindow.hpp"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QMimeData>

QWidget* SignedValueFactory::createEditor(int userType, QWidget *parent) const
{
    QSpinBox* sb = new QSpinBox(parent);
    sb->setFrame(false);
    sb->setMinimum(-128);
    sb->setMaximum(127);
    return sb;
}

QWidget* UnsignedValueFactory::createEditor(int userType, QWidget *parent) const
{
    QSpinBox* sb = new QSpinBox(parent);
    sb->setFrame(false);
    sb->setMinimum(0);
    sb->setMaximum(127);
    return sb;
}

EditorFieldProjectNode::EditorFieldProjectNode(ProjectModel::CollectionNode* collection, QWidget* parent)
: FieldProjectNode(collection, parent)
{}

SoundMacroDelegate::SoundMacroDelegate(QObject* parent)
: QStyledItemDelegate(parent) {}

QWidget* SoundMacroDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const LayersModel* model = static_cast<const LayersModel*>(index.model());
    ProjectModel::GroupNode* group = g_MainWindow->projectModel()->getGroupNode(model->m_node.get());
    EditorFieldProjectNode* cb =
        new EditorFieldProjectNode(group->getCollectionOfType(ProjectModel::INode::Type::SoundMacro), parent);
    connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(smIndexChanged()));
    return cb;
}

void SoundMacroDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    const LayersModel* model = static_cast<const LayersModel*>(index.model());
    const amuse::LayerMapping& layer = (*model->m_node->m_obj)[index.row()];
    ProjectModel::GroupNode* group = g_MainWindow->projectModel()->getGroupNode(model->m_node.get());
    ProjectModel::CollectionNode* smColl = group->getCollectionOfType(ProjectModel::INode::Type::SoundMacro);
    static_cast<EditorFieldProjectNode*>(editor)->setCurrentIndex(smColl->indexOfId(layer.macro.id) + 1);
    if (static_cast<EditorFieldProjectNode*>(editor)->shouldPopupOpen())
        static_cast<EditorFieldProjectNode*>(editor)->showPopup();
}

void SoundMacroDelegate::setModelData(QWidget* editor, QAbstractItemModel* m, const QModelIndex& index) const
{
    const LayersModel* model = static_cast<const LayersModel*>(m);
    amuse::LayerMapping& layer = (*model->m_node->m_obj)[index.row()];
    ProjectModel::GroupNode* group = g_MainWindow->projectModel()->getGroupNode(model->m_node.get());
    ProjectModel::CollectionNode* smColl = group->getCollectionOfType(ProjectModel::INode::Type::SoundMacro);
    int idx = static_cast<EditorFieldProjectNode*>(editor)->currentIndex();
    if (idx == 0)
        layer.macro.id = amuse::SoundMacroId();
    else
        layer.macro.id = smColl->idOfIndex(idx - 1);
}

void SoundMacroDelegate::smIndexChanged()
{
    emit commitData(static_cast<QWidget*>(sender()));
}

void LayersModel::loadData(ProjectModel::LayersNode* node)
{
    beginResetModel();
    m_node = node;
    endResetModel();
}

void LayersModel::unloadData()
{
    beginResetModel();
    m_node.reset();
    endResetModel();
}

int LayersModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    if (!m_node)
        return 0;
    return int(m_node->m_obj->size()) + 1;
}

int LayersModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return 8;
}

QVariant LayersModel::data(const QModelIndex& index, int role) const
{
    if (!m_node)
        return QVariant();
    if (index.row() == m_node->m_obj->size())
        return QVariant();
    const amuse::LayerMapping& layer = (*m_node->m_obj)[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column())
        {
        case 0:
        {
            ProjectModel::GroupNode* group = g_MainWindow->projectModel()->getGroupNode(m_node.get());
            ProjectModel::CollectionNode* smColl = group->getCollectionOfType(ProjectModel::INode::Type::SoundMacro);
            if (ProjectModel::BasePoolObjectNode* node = smColl->nodeOfId(layer.macro.id))
                return node->text();
            return QVariant();
        }
        case 1:
            return layer.keyLo;
        case 2:
            return layer.keyHi;
        case 3:
            return layer.transpose;
        case 4:
            return layer.volume;
        case 5:
            return layer.prioOffset;
        case 6:
            return layer.span;
        case 7:
            return layer.pan;
        default:
            break;
        }
    }

    return QVariant();
}

bool LayersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!m_node || role != Qt::EditRole)
        return false;
    amuse::LayerMapping& layer = (*m_node->m_obj)[index.row()];

    switch (index.column())
    {
    case 1:
        layer.keyLo = value.toInt();
        return true;
    case 2:
        layer.keyHi = value.toInt();
        return true;
    case 3:
        layer.transpose = value.toInt();
        return true;
    case 4:
        layer.volume = value.toInt();
        return true;
    case 5:
        layer.prioOffset = value.toInt();
        return true;
    case 6:
        layer.span = value.toInt();
        return true;
    case 7:
        layer.pan = value.toInt();
        return true;
    default:
        break;
    }

    return false;
}

QVariant LayersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return tr("SoundMacro");
        case 1:
            return tr("Key Lo");
        case 2:
            return tr("Key Hi");
        case 3:
            return tr("Transpose");
        case 4:
            return tr("Volume");
        case 5:
            return tr("Prio Off");
        case 6:
            return tr("Span");
        case 7:
            return tr("Pan");
        default:
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags LayersModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    if (index.row() == m_node->m_obj->size())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions LayersModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions LayersModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

bool LayersModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                               int row, int column, const QModelIndex& parent)
{
    // check if the action is supported
    if (!data || action != Qt::MoveAction)
        return false;
    // check if the format is supported
    QStringList types = mimeTypes();
    if (types.isEmpty())
        return false;
    QString format = types.at(0);
    if (!data->hasFormat(format))
        return false;

    // decode and insert
    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    std::unordered_set<int> rows;
    int lastRow = -1;

    while (!stream.atEnd()) {
        int r, c;
        QMap<int, QVariant> v;
        stream >> r >> c >> v;
        rows.insert(r);
        lastRow = r;
    }

    if (lastRow == -1)
        return false;

    int start = lastRow;
    while (rows.find(start - 1) != rows.cend())
        start -= 1;
    int count = lastRow - start + 1;
    while (rows.find(start + count) != rows.cend())
        count += 1;

    int dest = parent.row();
    if (dest >= start)
    {
        if (dest - start < count)
            return false;
        dest += 1;
    }

    moveRows(QModelIndex(), start, count, QModelIndex(), dest);
    return true;
}

bool LayersModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (!m_node)
        return false;
    beginInsertRows(parent, row, row + count - 1);
    std::vector<amuse::LayerMapping>& layers = *m_node->m_obj;
    layers.insert(layers.begin() + row, count, amuse::LayerMapping());
    endInsertRows();
    return true;
}

bool LayersModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                           const QModelIndex& destinationParent, int destinationChild)
{
    if (!m_node)
        return false;
    beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    std::vector<amuse::LayerMapping>& layers = *m_node->m_obj;
    if (destinationChild < sourceRow)
    {
        for (int i = 0; i < count; ++i)
        {
            amuse::LayerMapping tmp = std::move(layers[sourceRow]);
            for (int j = sourceRow; j != destinationChild; --j)
                layers[j] = std::move(layers[j - 1]);
            layers[destinationChild] = std::move(tmp);
            ++sourceRow;
            ++destinationChild;
        }
    }
    else if (destinationChild > sourceRow)
    {
        for (int i = 0; i < count; ++i)
        {
            amuse::LayerMapping tmp = std::move(layers[sourceRow]);
            for (int j = sourceRow; j != destinationChild - 1; ++j)
                layers[j] = std::move(layers[j + 1]);
            layers[destinationChild - 1] = std::move(tmp);
        }
    }
    endMoveRows();
    return true;
}

bool LayersModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (!m_node)
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    std::vector<amuse::LayerMapping>& layers = *m_node->m_obj;
    layers.erase(layers.begin() + row, layers.begin() + row + count);
    endRemoveRows();
    return true;
}

LayersModel::LayersModel(QObject* parent)
: QAbstractTableModel(parent)
{}

void LayersTableView::deleteSelection()
{
    QModelIndexList list;
    while (!(list = selectionModel()->selectedRows()).isEmpty())
        model()->removeRow(list.back().row());
}

void LayersTableView::doItemsLayout()
{
    horizontalHeader()->setMinimumSectionSize(75);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(1, 75);
    horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(2, 75);
    horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(3, 75);
    horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(4, 75);
    horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(5, 75);
    horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(6, 75);
    horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(7, 75);
    QTableView::doItemsLayout();
}

LayersTableView::LayersTableView(QWidget* parent)
: QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setDragEnabled(true);
    setGridStyle(Qt::NoPen);
}


bool LayersEditor::loadData(ProjectModel::LayersNode* node)
{
    m_model.loadData(node);
    return true;
}

void LayersEditor::unloadData()
{
    m_model.unloadData();
}

ProjectModel::INode* LayersEditor::currentNode() const
{
    return m_model.m_node.get();
}

void LayersEditor::resizeEvent(QResizeEvent* ev)
{
    m_tableView.setGeometry(QRect({}, ev->size()));
    m_addButton.move(0, ev->size().height() - 32);
    m_removeButton.move(32, ev->size().height() - 32);
}

void LayersEditor::doAdd()
{
    QModelIndex idx = m_tableView.selectionModel()->currentIndex();
    if (!idx.isValid())
        m_model.insertRow(m_model.rowCount() - 1);
    else
        m_model.insertRow(idx.row());
}

void LayersEditor::doSelectionChanged(const QItemSelection& selected)
{
    m_removeAction.setDisabled(selected.isEmpty());
    g_MainWindow->updateFocus();
}

bool LayersEditor::isItemEditEnabled() const
{
    return !m_tableView.selectionModel()->selectedRows().isEmpty();
}

void LayersEditor::itemCutAction()
{

}

void LayersEditor::itemCopyAction()
{

}

void LayersEditor::itemPasteAction()
{

}

void LayersEditor::itemDeleteAction()
{
    m_tableView.deleteSelection();
}

LayersEditor::LayersEditor(QWidget* parent)
: EditorWidget(parent), m_tableView(this),
  m_addAction(tr("Add Row")), m_addButton(this), m_removeAction(tr("Remove Row")), m_removeButton(this)
{
    m_signedDelegate.setItemEditorFactory(&m_signedFactory);
    m_unsignedDelegate.setItemEditorFactory(&m_unsignedFactory);

    m_tableView.setItemDelegateForColumn(1, &m_unsignedDelegate);
    m_tableView.setItemDelegateForColumn(2, &m_unsignedDelegate);
    m_tableView.setItemDelegateForColumn(3, &m_signedDelegate);
    m_tableView.setItemDelegateForColumn(4, &m_unsignedDelegate);
    m_tableView.setItemDelegateForColumn(5, &m_signedDelegate);
    m_tableView.setItemDelegateForColumn(6, &m_unsignedDelegate);
    m_tableView.setItemDelegateForColumn(7, &m_unsignedDelegate);

    m_tableView.setModel(&m_model);
    m_tableView.setItemDelegateForColumn(0, &m_smDelegate);
    connect(m_tableView.selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(doSelectionChanged(const QItemSelection&)));

    m_addAction.setIcon(QIcon(QStringLiteral(":/icons/IconAdd.svg")));
    m_addButton.setDefaultAction(&m_addAction);
    m_addButton.setFixedSize(32, 32);
    connect(&m_addAction, SIGNAL(triggered(bool)), this, SLOT(doAdd()));

    m_removeAction.setIcon(QIcon(QStringLiteral(":/icons/IconRemove.svg")));
    m_removeButton.setDefaultAction(&m_removeAction);
    m_removeButton.setFixedSize(32, 32);
    connect(&m_removeAction, SIGNAL(triggered(bool)), this, SLOT(itemDeleteAction()));
    m_removeAction.setEnabled(false);
}
