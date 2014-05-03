// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ratcoinUnits.h"

#include <QStringList>

CRatcoinUnits::CRatcoinUnits(QObject *parent):
		QAbstractListModel(parent),
		unitlist(availableUnits())
{
}

QList<CRatcoinUnits::Unit> CRatcoinUnits::availableUnits()
{
	QList<CRatcoinUnits::Unit> unitlist;
	unitlist.append(rat);
	unitlist.append(krat);
	unitlist.append(Mrat);
	unitlist.append(Grat);
	unitlist.append(Trat);
	return unitlist;
}

bool CRatcoinUnits::valid(int unit)
{
	switch(unit)
	{
	case rat:
	case krat:
	case Mrat:
	case Grat:
	case Trat:
		return true;
	default:
		return false;
	}
}

QString CRatcoinUnits::name(int unit)
{
	switch(unit)
	{
	case rat: return QString("Rat");
	case krat: return QString("KRat");
	case Mrat: return QString("MRat");
	case Grat: return QString("GRat");
	case Trat: return QString("TRat");
	default: return QString("???");
	}
}

QString CRatcoinUnits::description(int unit)
{
	switch(unit)
	{
	case rat: return QString("Ratcoins");
	case krat: return QString("Kilo-Ratcoins (1 * 10^3)");
	case Mrat: return QString("Mega-Ratcoins (1 * 10^6");
	case Grat: return QString("Giga-Ratcoins (1 * 10^9");
	case Trat: return QString("Terra-Ratcoins (1 * 10^12)");
	default: return QString("???");
	}
}

qint64 CRatcoinUnits::factor(int unit)
{
	switch(unit)
	{
	case rat:  return 1;
	case krat: return 1000;
	case Mrat: return 1000000;
	case Grat: return 1000000000;
	case Trat: return 1000000000000;
	default:   return 100000000;
	}
}

qint64 CRatcoinUnits::maxAmount(int unit)
{
	switch(unit)
	{
	case rat:  return Q_INT64_C(2100000000000000);
	case krat: return Q_INT64_C(2100000000000);
	case Mrat: return Q_INT64_C(2100000000);
	case Grat: return Q_INT64_C(2100000);
	case Trat: return Q_INT64_C(2100);
	default:   return 0;
	}
}

int CRatcoinUnits::amountDigits(int unit)
{
	switch(unit)
	{
	case rat:  return 16;
	case krat: return 13;
	case Mrat: return 10;
	case Grat: return 7;
	case Trat: return 4;
	default: return 0;
	}
}

int CRatcoinUnits::decimals(int unit)
{
	switch(unit)
	{
	case rat:  return 0;
	case krat: return 3;
	case Mrat: return 6;
	case Grat: return 9;
	case Trat: return 12;
	default: return 0;
	}
}

QString CRatcoinUnits::format(int unit, qint64 n, bool fPlus)
{
	// Note: not using straight sprintf here because we do NOT want
	// localized number formatting.
	if(!valid(unit))
		return QString(); // Refuse to format invalid unit
	qint64 coin = factor(unit);
	int num_decimals = decimals(unit);
	qint64 n_abs = (n > 0 ? n : -n);
	qint64 quotient = n_abs / coin;
	qint64 remainder = n_abs % coin;
	QString quotient_str = QString::number(quotient);
	QString remainder_str = QString::number(remainder).rightJustified(num_decimals, '0');

	// Right-trim excess zeros after the decimal point
	int nTrim = 0;
	for (int i = remainder_str.size()-1; i>=2 && (remainder_str.at(i) == '0'); --i)
		++nTrim;
	remainder_str.chop(nTrim);

	if (n < 0)
		quotient_str.insert(0, '-');
	else if (fPlus && n > 0)
		quotient_str.insert(0, '+');
	return quotient_str + QString(".") + remainder_str;
}

QString CRatcoinUnits::formatWithUnit(int unit, qint64 amount, bool plussign)
{
	return format(unit, amount, plussign) + QString(" ") + name(unit);
}

bool CRatcoinUnits::parse(int unit, const QString &value, qint64 *val_out)
{
	if(!valid(unit) || value.isEmpty())
		return false; // Refuse to parse invalid unit or empty string
	int num_decimals = decimals(unit);
	QStringList parts = value.split(".");

	if(parts.size() > 2)
	{
		return false; // More than one dot
	}
	QString whole = parts[0];
	QString decimals;

	if(parts.size() > 1)
	{
		decimals = parts[1];
	}
	if(decimals.size() > num_decimals)
	{
		return false; // Exceeds max precision
	}
	bool ok = false;
	QString str = whole + decimals.leftJustified(num_decimals, '0');

	if(str.size() > 18)
	{
		return false; // Longer numbers will exceed 63 bits
	}
	qint64 retvalue = str.toLongLong(&ok);
	if(val_out)
	{
		*val_out = retvalue;
	}
	return ok;
}

int CRatcoinUnits::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return unitlist.size();
}

QVariant CRatcoinUnits::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	if(row >= 0 && row < unitlist.size())
	{
		Unit unit = unitlist.at(row);
		switch(role)
		{
		case Qt::EditRole:
		case Qt::DisplayRole:
			return QVariant(name(unit));
		case Qt::ToolTipRole:
			return QVariant(description(unit));
		case UnitRole:
			return QVariant(static_cast<int>(unit));
		}
	}
	return QVariant();
}

